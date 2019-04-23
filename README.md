### MSL-Hydra-Synth-Player

Play the [MSL-Hydra-Synth](https://github.com/mizt/MSL-Hydra-Synth-Player)

The following two files uses [https://hydra-editor-v1.glitch.me/?sketch_id=example_14&code=JTJGJTJGJTIwYnklMjBPbGl2aWElMjBKYWNrJTBBJTJGJTJGJTIwJTQwX29qYWNrXyUwQSUwQW9zYygyMCUyQyUyMDAuMDElMkMlMjAxLjEpJTBBJTA5LmthbGVpZCg1KSUwQSUwOS5jb2xvcigyLjgzJTJDMC45MSUyQzAuMzkpJTBBJTA5LnJvdGF0ZSgwJTJDJTIwMC4xKSUwQSUwOS5tb2R1bGF0ZShvMCUyQyUyMCgpJTIwJTNEJTNFJTIwbW91c2UueCUyMColMjAwLjAwMDMpJTBBJTA5LnNjYWxlKDEuMDEpJTBBJTIwJTIwJTA5Lm91dChvMCk=)](https://hydra-editor-v1.glitch.me/?sketch_id=example_14&code=JTJGJTJGJTIwYnklMjBPbGl2aWElMjBKYWNrJTBBJTJGJTJGJTIwJTQwX29qYWNrXyUwQSUwQW9zYygyMCUyQyUyMDAuMDElMkMlMjAxLjEpJTBBJTA5LmthbGVpZCg1KSUwQSUwOS5jb2xvcigyLjgzJTJDMC45MSUyQzAuMzkpJTBBJTA5LnJvdGF0ZSgwJTJDJTIwMC4xKSUwQSUwOS5tb2R1bGF0ZShvMCUyQyUyMCgpJTIwJTNEJTNFJTIwbW91c2UueCUyMColMjAwLjAwMDMpJTBBJTA5LnNjYWxlKDEuMDEpJTBBJTIwJTIwJTA5Lm91dChvMCk=)

* ./assts/u0.json
* ./assts/s0.metallib

### Build Command Line Tool & Run

	$ xcrun clang++ -ObjC++ -lc++ -fobjc-arc -O3 -std=c++17 -Wc++17-extensions -framework Cocoa -framework Metal -framework Quartz -framework JavascriptCore ./libs/MetalView.mm ./Hydra-Synth-Player.mm -o ./Hydra-Synth-Player
	$ ./Hydra-Synth-Player
