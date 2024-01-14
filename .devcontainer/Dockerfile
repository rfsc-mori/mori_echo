FROM mcr.microsoft.com/devcontainers/cpp

ARG DEVCONTAINER_USER=vscode

RUN mkdir -p /home/${DEVCONTAINER_USER}/.config/fish && \
    chown ${DEVCONTAINER_USER}:${DEVCONTAINER_USER} /home/${DEVCONTAINER_USER}/.config/fish && \
    chmod 700 /home/${DEVCONTAINER_USER}/.config && \
    chmod 700 /home/${DEVCONTAINER_USER}/.config/fish

RUN mkdir -p /home/${DEVCONTAINER_USER}/workspace && \
    chown ${DEVCONTAINER_USER}:${DEVCONTAINER_USER} /home/${DEVCONTAINER_USER}/workspace && \
    chmod 700 /home/${DEVCONTAINER_USER}/workspace

USER ${DEVCONTAINER_USER}
