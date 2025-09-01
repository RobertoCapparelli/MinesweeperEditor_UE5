**Engine**: Unreal Engine 5.4 
**Install as Project Plugin**: <ProjectRoot>/Plugins/
**Shortcut**: Ctrl+Shift+M

**Code Structure**

- Board (FMinesweeperBoard): Game logic with no Slate dependency.
- BoardView (SMinesweeperBoardView): Rendering and mouse input handling.
- Window (SMinesweeperWindow): Hosts the board and controls.
- Editor Module (MinesweeperEditor, MinesweeperEditorCommands): Tab registration, menu and toolbar integration.


**AI and LLM**

- Used to accelerate time-consuming parts.
- OnPaint, Specific help on SMinesweeperBoardView::OnPaint.
- Brainstorming, Helped decide the tool structure and optimizations (caching).
- Code review, Used during development as a reviewer.


**Implementation Notes**

- Single Slate widget (SLeafWidget), Draws the grid in OnPaint, avoiding hundreds of per-cell widgets.
- Flood-fill (BFS), Implemented iteratively with a queue.
- First-move safe, The first reveal cannot explode; if it hits a bomb, the bomb is relocated and adjacency is recomputed.
- Centralized clamping, Parameters clamped in Limits.
- Editor notifications, Start, win, and loss.
