import subprocess
import time
import os
from concurrent.futures import ThreadPoolExecutor, as_completed


class ImgEngineBenchmark:
    def __init__(self, binary_path, input_file, output_dir,
                 runs=1000, workers=1, extra_args=None):
        self.binary = binary_path
        self.input = input_file
        self.output_dir = output_dir
        self.runs = runs
        self.workers = workers
        self.extra_args = extra_args or []

        os.makedirs(self.output_dir, exist_ok=True)

    def _build_command(self, index):
        output_file = os.path.join(self.output_dir, f"photo_{index}.jpg")

        cmd = [
            self.binary,
            "--input", self.input,
            "--output", output_file,
        ] + self.extra_args

        return cmd

    def _run_once(self, index):
        cmd = self._build_command(index)

        start = time.time()

        try:
            result = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )

            end = time.time()
            duration = end - start

            return {
                "index": index,
                "success": result.returncode == 0,
                "time": duration,
                "stderr": result.stderr.decode()
            }

        except Exception as e:
            return {
                "index": index,
                "success": False,
                "time": 0,
                "stderr": str(e)
            }

    def run(self):
        print(f"🔥 Running {self.runs} jobs with {self.workers} workers...\n")

        start_total = time.time()
        results = []

        with ThreadPoolExecutor(max_workers=self.workers) as executor:
            futures = [executor.submit(self._run_once, i)
                       for i in range(1, self.runs + 1)]

            for f in as_completed(futures):
                res = f.result()
                results.append(res)

                if res["success"]:
                    print(f"✔ {res['index']} | {res['time']:.4f}s")
                else:
                    print(f"❌ {res['index']} | ERROR: {res['stderr']}")

        end_total = time.time()

        self._report(results, end_total - start_total)

    def _report(self, results, total_time):
        success = [r for r in results if r["success"]]
        failed = [r for r in results if not r["success"]]

        if success:
            avg_time = sum(r["time"] for r in success) / len(success)
        else:
            avg_time = 0

        throughput = len(success) / total_time if total_time > 0 else 0

        print("\n" + "=" * 50)
        print("📊 BENCHMARK RESULT")
        print("=" * 50)
        print(f"Total Runs     : {len(results)}")
        print(f"Success        : {len(success)}")
        print(f"Failed         : {len(failed)}")
        print(f"Total Time     : {total_time:.4f}s")
        print(f"Avg Time       : {avg_time:.4f}s")
        print(f"Throughput     : {throughput:.2f} ops/sec")
        print("=" * 50)


# 🚀 ENTRY POINT
if __name__ == "__main__":
    bench = ImgEngineBenchmark(
        binary_path="../build/imgengine_cli",
        input_file="../photo.jpg",
        output_dir="./outputs",
        runs=1000,
        workers=4,  # 🔥 increase to stress test
        extra_args=[
            "--cols", "6",
            "--rows", "3",
            "--gap", "15",
            "--padding", "20",
            "--bleed", "10",
            "--crop-mark", "25",
            "--crop-offset", "8",
            "--crop-thickness", "2"
        ]
    )

    bench.run()