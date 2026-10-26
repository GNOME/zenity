# Contributing

Thank you for considering contributing to Zenity.

These guidelines are meant for new contributors, regardless of their
level of proficiency; following them allows other contributors to Zenity
to more effectively evaluate your contribution, and provide prompt
feedback to you. Additionally, by following these guidelines you clearly
communicate that you respect the time and effort that the people working
on Zenity put into managing the project.

# Code of Conduct

Although Zenity is not a core GNOME application, we fully adopt the
GNOME Code of Conduct.

As such, when interacting with the project, the GNOME Code of Conduct
applies.

# Code Style

We use tabs, not spaces, as our indentation, with a tabstop of 4
characters.

Zenity mostly follows the Allman style of C indentation. That said,
there are times that keeping the `{` on the same line as the condition
line looks cleaner (ie, K&R style). We are not dogmatic about this. The
best approach is to try to match the style of surrounding code
throughout the codebase.

# Versioning

As Zenity is not a core GNOME application, or truly an "application" in
the traditional sense of the word, we follow a more traditional "major,
minor, patchlevel" versioning scheme.

When it comes to the minor version number, even numbers represent a
stable release and odd numbers represent unstable releases. As well, for
alpha, beta, etc. pre-releases, the "Numeric 90+" system is utilized.

Since Zenity makes use of a well-defined command-line interface, this
creates a form of API even though it is not a library in the traditional
sense. What this means is, if an option is introduced that was not there
before, users of older versions of Zenity will not be able to run that
script.

As such, only in new minor releases of Zenity do we consider adding new
command-line options. Even then, these additional options must be
considered carefully, because once they are there, they become expected
to be there, and it is hard to "unring the bell."

# AI Contribution Policy

*The below policy is based on the GTK AI Contribution Policy.*

Zenity is a project by humans for humans. We prefer contributions that
are produced by human creativity, we expect a human to take full
responsibility for each contribution, and we will take more joy in
reviewing contributions when there's people at the other end of the
line to stand by their changes.

That being said, we do not impose an absolute ban on AI contributions,
so long as our rules are followed.

Merge requests submitted after this policy was instituted that do not
follow these rules will be closed immediately. Repeated rule violations
may result in a summary banning of future contributions from the
contributor in question, at the sole discretion of the maintainer(s) of
Zenity.

If you use LLM/GenAI tools for your contributions, here are the rules
you must follow:

## Requirements

Use AI as a tool. Verify behavior, correctness, and compatibility
yourself prior to submitting your contribution. Do not ask the
maintainers to do this for you.

1. Keep changes narrow and limited. Do **NOT** use LLM/GenAI tools to
   generate broad rewrites, large refactorings, or style changes.
1. Do **NOT** submit generated code, documentation, or tests that you
   don’t understand.
1. Do **NOT** fabricate benchmarks, bug reports, test results, code
   samples, or reproducers.
1. Do **NOT** include private code, credentials, tokens, or any other
   confidential material.
1. Respect the licensing and attribution requirements.

## Disclosure

***Always*** disclose the use of LLM/GenAI tools when creating an issue or
a merge request. Do not include trailers like “Co-authored-by:” or
“Assisted-by:” in commit messages, since they serve as free advertising
for AI companies.

## Reviews

Describe your changes, and the verification steps.
Be prepared to explain all the changes yourself.

Do NOT feed the review feedback to an LLM/GenAI tool.

## Maintainers' Expectations

1. Review LLM/GenAI-assisted contributions more strictly than any other
   contribution.
1. Require reproducibility in fixes and tests.
1. Reject changes that appear to be unverified LLM/GenAI output.
1. Reject comments and feedback that appear to be LLM/GenAI output.

> A COMPUTER CAN NEVER BE HELD ACCOUNTABLE.
> THEREFORE A COMPUTER MUST NEVER MAKE A MAINTENANCE DECISION.
