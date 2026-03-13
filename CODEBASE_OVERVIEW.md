# Unfold Codebase Overview

Unfold is a spatial file explorer built with Qt6 that visualizes a filesystem as an interactive node graph on an infinite zoomable canvas. This document explains the project's structure and the role of each code file.

## Directory Structure

```text
UnFold/
├── resources/          # Static assets and Qt resource definitions
├── src/                # C++ Source and Header files
│   ├── app/            # Application-level UI and main logic
│   ├── canvas/         # Graphics scene and view implementation
│   ├── filesystem/     # Filesystem operations and monitoring
│   ├── items/          # Graphics items (Nodes and Edges)
│   ├── layout/         # Graph layout algorithms
│   └── persistence/    # Workspace and configuration management
├── CMakeLists.txt      # Build configuration
└── README.md           # Project introduction and build instructions
```

---

## Core Components (src/)

### 1. Application Layer (`src/app/`)
This module manages the main window, toolbars, and global application state.

- **[main.cpp](file:///home/ashish/Code/Projects/UnFold/src/main.cpp)**: Entry point of the application. Initializes the `QApplication`, sets the "Fusion" dark theme, and launches the `MainWindow`.
- **[MainWindow](file:///home/ashish/Code/Projects/UnFold/src/app/MainWindow.h)**: The primary container for the UI. It hosts the spatial view, toolbars, status bar, and coordinates between various managers (filesystem, workspace, settings).
- **[MiniMap](file:///home/ashish/Code/Projects/UnFold/src/app/MiniMap.h)**: Provides a birds-eye view of the entire spatial canvas for quick navigation.
- **[SettingsView](file:///home/ashish/Code/Projects/UnFold/src/app/SettingsView.h)**: UI for application-wide settings, such as toggling hidden files or setting default paths.

### 2. Canvas Layer (`src/canvas/`)
Implements the infinite zoomable workspace using Qt's Graphics View Framework.

- **[SpatialScene](file:///home/ashish/Code/Projects/UnFold/src/canvas/SpatialScene.h)**: A specialized `QGraphicsScene` that manages the graph structure. It handles node creation, expansion/collapse logic, and coordinates with the layout engine.
- **[SpatialView](file:///home/ashish/Code/Projects/UnFold/src/canvas/SpatialView.h)**: A `QGraphicsView` subclass that enables infinite panning and zooming on the scene.

### 3. Graphics Items (`src/items/`)
Defines the visual representation of files and their relationships.

- **[NodeItem](file:///home/ashish/Code/Projects/UnFold/src/items/NodeItem.h)**: Represents a file or directory as a node in the graph. Handles its own painting, interaction (click, double-click), and state (expanded/collapsed).
- **[EdgeItem](file:///home/ashish/Code/Projects/UnFold/src/items/EdgeItem.h)**: Represents the connection between a parent directory and its children.

### 4. Filesystem Layer (`src/filesystem/`)
Handles asynchronous file operations to keep the UI responsive.

- **[FilesystemWorker](file:///home/ashish/Code/Projects/UnFold/src/filesystem/FilesystemWorker.h)**: Manages a background thread (`DirectoryReader`) for listing directory contents without blocking the main event loop.
- **[FileWatcher](file:///home/ashish/Code/Projects/UnFold/src/filesystem/FileWatcher.h)**: Monitors the filesystem for changes (additions, deletions) and notifies the scene to update accordingly.

### 5. Layout Engine (`src/layout/`)
Manages the positioning of nodes in the spatial workspace.

- **[LayoutEngine](file:///home/ashish/Code/Projects/UnFold/src/layout/LayoutEngine.h)**: Implements algorithms for arranging nodes (radial and grid layouts). It also handles collision detection and resolution when nodes are moved or expanded.

### 6. Persistence (`src/persistence/`)
Manages saving and loading of application state.

- **[WorkspaceManager](file:///home/ashish/Code/Projects/UnFold/src/persistence/WorkspaceManager.h)**: Responsible for saving and restoring the entire graph state (node positions, expansion states) to/from files.
- **[SettingsManager](file:///home/ashish/Code/Projects/UnFold/src/persistence/SettingsManager.h)**: Handles persistent application settings using `QSettings`.

---

## Static Resources (`resources/`)

- **[resources.qrc](file:///home/ashish/Code/Projects/UnFold/resources/resources.qrc)**: Compiled Qt resource file containing references to icons and other assets.
- **[icons/](file:///home/ashish/Code/Projects/UnFold/resources/icons/)**: Contains SVG and PNG icons used throughout the application.
- **[app.rc](file:///home/ashish/Code/Projects/UnFold/resources/app.rc)**: Windows-specific resource file for application metadata (icons, versioning).
