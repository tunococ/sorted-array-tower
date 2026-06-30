# Import cmake.just if it exists. Remove this line if you don't use CMake.
import? 'cmake.just'

# Clean all artifacts from the build system
clean:
    rm -rf build CMakeUserPresets.json .xmake


# Docker commands
# ===============

# Create an ephemeral container and run an interactive shell inside it.
run-docker variant='alpine' stage='full' *args='':
    VARIANT={{ variant }} docker compose -f ci/compose.yaml \
        run --rm -it {{ stage }} {{ args }}

# Create a new docker container.
create-docker variant='alpine' stage='full' name=(variant + '-' + stage) \
    *args='tail -f /dev/null':
    docker rm -f {{ name }} 2>/dev/null || true
    VARIANT={{ variant }} docker compose -f ci/compose.yaml \
        run -d --name {{ name }} {{ stage }} {{ args }}

# Remove Docker images matching a given prefix.
clean-docker-images prefix='sorted_array_tower-':
    docker rmi $(docker images --format '{{{{.Repository}}:{{{{.Tag}}' | \
        grep '^{{ prefix }}') 2>/dev/null || true


# Documentation
# =============

# Generate documentation with Doxygen
doc:
    mkdir -p build/doc
    doxygen Doxyfile

# Remove generated documentation
clean-doc:
    rm -rf build/doc

# Show documentation in a web browser
show-doc port='8060':
    python3 -m http.server --directory build/doc {{ port }}

# Clean the documentation directory, rebuild it, and display it.
docx port='8060':
    just clean-doc
    just doc
    just show-doc {{ port }}


