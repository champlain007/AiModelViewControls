#!/bin/bash
set -e

# This is the entrypoint for the official AgenticMVCpipe container.
# It ensures that the mandatory 'tradesecret.sh' script is used to boot the node.

# The VOLUME instruction in the Dockerfile maps the 'data' directory.
# We need to ensure our application looks for the blockchain ledger there.
# We can do this by setting a specific environment variable that the C++
# code will respect.

export AGENTIC_DATA_DIR=/home/agent/app/data

# Execute the main boot script, passing through any commands sent to `docker run`
exec ./tradesecret.sh "$@"
