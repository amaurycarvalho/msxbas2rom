/***
 * @file runner.h
 * @brief MSXBAS2ROM CLI runner entry point
 * @author Amaury Carvalho
 */

#ifndef RUNNER_H_INCLUDED
#define RUNNER_H_INCLUDED

/***
 * @brief Execute the CLI with the supplied arguments.
 *
 * This is the composition root: it constructs the production adapters and
 * passes them explicitly to the application workflow.
 */
int runCli(int argc, char* argv[]);

#endif  // RUNNER_H_INCLUDED
