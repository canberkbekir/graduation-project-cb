# graduation-project-cb

## Git LFS

This repository uses [Git Large File Storage (LFS)](https://git-lfs.github.com/) to manage large binary assets such as `.uasset`, `.umap`, `.fbx`, `.png`, `.tga`, `.wav`, `.mp3`, and `.ogg` files.

Unreal Engine projects contain binary asset files that can easily reach hundreds of megabytes. GitHub enforces a 100 MB per-file limit and rejects pushes containing files larger than that. To work around this, Git LFS stores the actual binary data on a separate LFS server while keeping lightweight pointer files in the repository. This allows the full project — including large MetaHuman and texture assets — to be versioned and shared on GitHub without hitting size limits.

### Setup

Make sure Git LFS is installed before cloning or pulling:

```bash
git lfs install
git clone https://github.com/canberkbekir/graduation-project-cb.git
```
