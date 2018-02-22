Git/GitHub workflow
---------------
* Commits to `master` may be:
	* Merge with feature branch (branch that contains new feature, or big bug fixing) - feature branch must be avaliable on GitHub; after merging it must be deleted
	* Little and clean bug fixes
	* Documentation updates
	* Build scripting updates (may be done in a feature branch, by author's choice)
	* Independent resources updates

* Creating feature branches and committing to them is free
* Special merge requests, not for merging or with delayed time for merging must contain the `[NOT_FOR_MERGING]` prefix; such requests may be merged only after request's author writes comment `[CAN_BE_MERGED_NOW]`
* 
* How to submit a feature branch to `master`:
	1. Install GIT bash and launch the terminal (or use your own GUI)
	2. Create a local feature branch by issuing: `git checkout -b feature_branch_name` (or make one in the repo with GitHub's web UI)
	3. Commit the local branch with `git add -u`, `git commit` (if you use Vim, press 'i', enter message, press escape, then enter)
	4. Merge the feature branch with the local master branch with `git merge master` and resolve any conflict (rebuild the project)
	5. Commit the changes and push the local feature branch to the remote on GitHub with: `git push origin feature_branch_name`
	6. Create a merge (pull) request with the GitHub web UI
	7. After merging is accepted, delete the feature branch

* _extern_ folder is not to be edited - it's a place for external libraries sources/headers
* _src/core_ folder: only by TeslaRus, make request if you want to change something
* Other folders: by merge requests to TeslaRus or, after code review (by merge request) by command (more details will be discussed); I will make some commits after merge request too (number of errors will be decreased significantly in case of review)