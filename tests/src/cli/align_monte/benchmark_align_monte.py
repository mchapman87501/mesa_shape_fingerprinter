import subprocess
import time

import config

exe = config.EXE
sd_path = config.TEST_DATA_DIR / "cox2_3d.sd"
spheroid_path = (
    config.SHARED_TEST_DATA_DIR / "hammersley" / "hamm_spheroid_20k_11rad.txt"
)


def run_benchmark_once() -> None:
    subprocess.check_call(
        [exe, sd_path, spheroid_path, "1.0"],
        stdout=subprocess.DEVNULL,
        # stderr=subprocess.DEVNULL,
    )


def run_benchmark(num_runs: int) -> float:
    t0 = time.time()
    for i in range(num_runs):
        run_benchmark_once()
    return (time.time() - t0) / num_runs


def main() -> None:
    mean = run_benchmark(20)
    print(f"Mean runtime: {mean:.2f} seconds")


if __name__ == "__main__":
    main()
