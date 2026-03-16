# HELP: AiModelViewControls (AiMVCs) Troubleshooting

## Handshake Failures
If you receive a `[SECURITY_FATAL]` error, you are likely trying to run a binary directly. You MUST use `tradesecret.sh` to initialize the environment and provide the `TRADESECRET_HANDSHAKE_KEY`.

## Connection Pipelines
Pipelines are created via the `AgenticMVCpipe` sub-component. Ensure that all nodes in a pipeline are initialized with the same handshake key via the boot script.

## Security Heuristics
The framework includes native C++ Red Team detection. If a prompt is blocked, check the local CLI console or the Orchestrator alert log (if `--alert-sync` is enabled) for the specific violation code.
