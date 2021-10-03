# Scorpion

Scorpion is an optimal classical planner that uses saturated cost
partitioning to combine multiple abstraction heuristics. It also contains
implementations of many other cost partitioning algorithms over
abstraction and landmark heuristics. Scorpion is based on the [Fast
Downward planning system](https://github.com/aibasel/downward), which is
described below. We regularly port the latest changes from Fast Downward
to Scorpion and also try to port Scorpion features back to Fast Downward.

Please use the following reference when citing Scorpion:
Jendrik Seipp, Thomas Keller and Malte Helmert.
[Saturated Cost Partitioning for Optimal Classical Planning](
https://www.jair.org/index.php/jair/article/view/11673).
Journal of Artificial Intelligence Research 67, pp. 129-167. 2020.


## Instructions

After installing the requirements (see below), compile the planner with

    ./build.py

and see the available options with

    ./fast-downward.py --help  # driver
    ./fast-downward.py --search -- --help  # search component

For more details (including build instructions for Windows), see the
documentation about
[compiling](https://www.fast-downward.org/ObtainingAndRunningFastDownward)
and [running](https://www.fast-downward.org/PlannerUsage) the planner. The
[plugin documentation](https://jendrikseipp.github.io/scorpion) shows
which plugins are available (heuristics, search algorithms, etc.) and how
to use them.


### Recommended configuration

For state-of-the-art performance, we recommend the following
configuration, which is similar to the one Scorpion used in the IPC 2018.

```
./fast-downward.py \
  --transform-task preprocess-h2 \
  --alias scorpion \
  ../benchmarks/gripper/prob01.pddl
```

The `preprocess-h2` call prunes irrelevant operators in a preprocessing
step, and the `scorpion` alias uses partial order reduction and maximizes
over multiple diverse SCP heuristics computed for hillclimbing PDBs,
systematic PDBs and Cartesian abstractions (see
[aliases.py](driver/aliases.py) for the expanded configuration string).
(In [Downward Lab](https://lab.readthedocs.io/) you can use
`driver_options=["--transform-task", "preprocess-h2"]` in the call to
`add_algorithm` to prune irrelevant operators.)

#### Singularity container

To simplify the installation process, we provide the above Scorpion
configuration as an executable
[Singularity](https://github.com/hpcng/singularity) container.

Download and run the container (tested with Singularity 3.5):

    singularity pull scorpion.sif library://jendrikseipp/default/scorpion:latest
    ./scorpion.sif [domain_file] problem_file

Build the container yourself:

    sudo singularity build scorpion.sif Singularity

### IPC 2018 version

If you prefer to run exactly the same Scorpion version as in IPC 2018, we
recommend using the [Scorpion IPC
repo](https://bitbucket.org/ipc2018-classical/team44/src/ipc-2018-seq-opt/).


## Differences between Scorpion and Fast Downward

- Scorpion comes with the
  [h²-preprocessor](https://ojs.aaai.org/index.php/ICAPS/article/view/13708)
  by Vidal Alcázar and Álvaro Torralba that prunes irrelevant operators.
  Pass `--transform-task preprocess-h2` to use it.
- The `--transform-task` command allows you to run arbitrary preprocessing
  commands that transform the SAS+ output from the translator before
  passing it to the search.
- If [ccache](https://ccache.dev/) is installed (recommended), Scorpion
  uses it to cache compilation files.


### New plugin options

- `cegar(..., search_strategy=incremental)`: use [incremental search for
  Cartesian abstraction
  refinement](https://ojs.aaai.org/index.php/ICAPS/article/view/6667)
  (default).

- `hillclimbing(..., max_generated_patterns=200)`: limit the number of
  patterns generated by hill climbing.


### New cost partitioning algorithms for abstraction heuristics

We use Cartesian abstractions in the example configurations below
(`[cartesian()]`). You can also use pattern database heuristics, e.g.,
`[projections(systematic(2))]`, or mix abstractions, e.g.,
`[projections(systematic(3)), cartesian()]`. Some of the algorithms are
also part of vanilla Fast Downward, but only for PDB heuristics.

- Optimal cost partitioning:
  `optimal_cost_partitioning([cartesian()])`
- Canonical heuristic:
  `canonical_heuristic([cartesian()])`
- Post-hoc optimization:
  `operatorcounting([pho_abstraction_constraints([cartesian()], saturated=false)])`
- Uniform cost partitioning:
  `uniform_cost_partitioning([cartesian()], opportunistic=false)`
- Opportunistic uniform cost partitioning:
  `uniform_cost_partitioning([cartesian()], ..., opportunistic=true)`
- Greedy zero-one cost partitioning:
  `zero_one_cost_partitioning([cartesian()], ...)`
- Saturated post-hoc optimization:
  `operatorcounting([pho_abstraction_constraints([cartesian()], saturated=true)])`

You can also compute the maximum over abstraction heuristics:

- `maximize([cartesian()])`


### New cost partitioning algorithms for landmark heuristics

Example using A* search and saturated cost partitioning over BJOLP
landmarks:

    --evaluator
      "lmc=lmcount(lm_merged([lm_rhw(), lm_hm(m=1)]),
      admissible=true, cost_partitioning=suboptimal, greedy=true,
      reuse_costs=true, scoring_function=max_heuristic_per_stolen_costs)"
    --search
      "astar(lmc, lazy_evaluator=lmc)"

Different cost partitioning algorithms (all need `admissible=true`):

- Optimal cost partitioning (part of vanilla Fast Downward):
  `lmcount(..., cost_partitioning=optimal)`
- Canonical heuristic:
  `lmcount(..., cost_partitioning=canonical)`
- Post-hoc optimization:
  `lmcount(..., cost_partitioning=pho)`
- Uniform cost partitioning:
  `lmcount(..., cost_partitioning=suboptimal, greedy=false, reuse_costs=false)`
- Opportunistic uniform cost partitioning (part of vanilla Fast Downward):
  `lmcount(..., cost_partitioning=suboptimal, greedy=false, reuse_costs=true, scoring_function=min_stolen_costs)`
- Greedy zero-one cost partitioning:
  `lmcount(..., cost_partitioning=suboptimal, greedy=true, reuse_costs=false, scoring_function=max_heuristic)`
- Saturated cost partitioning:
  `lmcount(..., cost_partitioning=suboptimal, greedy=true, reuse_costs=true, scoring_function=max_heuristic_per_stolen_costs)`


<img src="misc/images/fast-downward.svg" width="800" alt="Fast Downward">

Fast Downward is a domain-independent classical planning system.

Copyright 2003-2020 Fast Downward contributors (see below).

For further information:
- Fast Downward website: <http://www.fast-downward.org>
- Report a bug or file an issue: <http://issues.fast-downward.org>
- Fast Downward mailing list: <https://groups.google.com/forum/#!forum/fast-downward>
- Fast Downward main repository: <https://github.com/aibasel/downward>


## Tested software versions

This version of Fast Downward has been tested with the following software versions:

| OS           | Python | C++ compiler                                                     | CMake |
| ------------ | ------ | ---------------------------------------------------------------- | ----- |
| Ubuntu 20.04 | 3.8    | GCC 9, GCC 10, Clang 10, Clang 11                                | 3.16  |
| Ubuntu 18.04 | 3.6    | GCC 7, Clang 6                                                   | 3.10  |
| macOS 10.15  | 3.6    | AppleClang 12                                                    | 3.19  |
| Windows 10   | 3.6    | Visual Studio Enterprise 2017 (MSVC 19.16) and 2019 (MSVC 19.28) | 3.19  |

We test LP support with CPLEX 12.9, SoPlex 3.1.1 and Osi 0.107.9.
On Ubuntu, we test both CPLEX and SoPlex. On Windows, we currently
only test CPLEX, and on macOS, we do not test LP solvers (yet).


## Contributors

The following list includes all people that actively contributed to
Fast Downward, i.e. all people that appear in some commits in Fast
Downward's history (see below for a history on how Fast Downward
emerged) or people that influenced the development of such commits.
Currently, this list is sorted by the last year the person has been
active, and in case of ties, by the earliest year the person started
contributing, and finally by last name.

- 2003-2020 Malte Helmert
- 2008-2016, 2018-2020 Gabriele Roeger
- 2010-2020 Jendrik Seipp
- 2010-2011, 2013-2020 Silvan Sievers
- 2012-2020 Florian Pommerening
- 2013, 2015-2020 Salome Eriksson
- 2016-2020 Cedric Geissmann
- 2017-2020 Guillem Francès
- 2018-2020 Augusto B. Corrêa
- 2018-2020 Patrick Ferber
- 2015-2019 Manuel Heusner
- 2017 Daniel Killenberger
- 2016 Yusra Alkhazraji
- 2016 Martin Wehrle
- 2014-2015 Patrick von Reth
- 2015 Thomas Keller
- 2009-2014 Erez Karpas
- 2014 Robert P. Goldman
- 2010-2012 Andrew Coles
- 2010, 2012 Patrik Haslum
- 2003-2011 Silvia Richter
- 2009-2011 Emil Keyder
- 2010-2011 Moritz Gronbach
- 2010-2011 Manuela Ortlieb
- 2011 Vidal Alcázar Saiz
- 2011 Michael Katz
- 2011 Raz Nissim
- 2010 Moritz Goebelbecker
- 2007-2009 Matthias Westphal
- 2009 Christian Muise


## History

The current version of Fast Downward is the merger of three different
projects:

- the original version of Fast Downward developed by Malte Helmert
  and Silvia Richter
- LAMA, developed by Silvia Richter and Matthias Westphal based on
  the original Fast Downward
- FD-Tech, a modified version of Fast Downward developed by Erez
  Karpas and Michael Katz based on the original code

In addition to these three main sources, the codebase incorporates
code and features from numerous branches of the Fast Downward codebase
developed for various research papers. The main contributors to these
branches are Malte Helmert, Gabi Röger and Silvia Richter.


## License

The following directory is not part of Fast Downward as covered by
this license:

- ./src/search/ext

For the rest, the following license applies:

```
Fast Downward is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

Fast Downward is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```
