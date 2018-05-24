#!/bin/bash

# deconf is really only meant for development, so it's allowed to rely on git.
# Also, I don't want to enumerate the autotools pollution files again;
# they are already listed in the global gitignore.
git clean -dfX
