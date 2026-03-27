import asyncio
import httpx
import time

# Configuration
URL = "http://127.0.0.1:8000/api/generate"
HEADERS = {"accept": "application/json", "x-api-key": "test-key-123"}
FILE_PATH = "test.jpg"  # Ensure this file exists in your current folder
TOTAL_REQUESTS = 1000
# CONCURRENT_LIMIT = 20  # How many to send at the exact same time

# To stay under "5 per minute", we must wait 12 seconds between requests
# (60 seconds / 5 = 12)
WAIT_TIME = 12.1


async def main():
    async with httpx.AsyncClient() as client:
        print(f"🚀 Starting slow stress test: 1 request every {WAIT_TIME}s")

        for i in range(TOTAL_REQUESTS):
            with open("4.jpg", "rb") as f:
                files = {"file": ("test.jpg", f, "image/jpeg")}
                response = await client.post(URL, headers=HEADERS, files=files)

            if response.status_code == 200:
                print(f"[{i}] ✅ {response.json()['job_id']}")
            elif response.status_code == 429:
                print(f"[{i}] ⚠️ RATE LIMITED! Sleeping extra...")
                await asyncio.sleep(60)  # Wait a full minute if we hit the wall

            await asyncio.sleep(WAIT_TIME)


if __name__ == "__main__":
    asyncio.run(main())


# Below code for Recruters show case


# import asyncio
# import httpx
# import time

# # Configuration
# URL = "http://127.0.0.1:8000/api/generate"
# HEADERS = {"accept": "application/json", "x-api-key": "test-key-123"}
# FILE_PATH = "4.jpg"  # Ensure this file exists in your current folder
# TOTAL_REQUESTS = 1000
# CONCURRENT_LIMIT = 20  # How many to send at the exact same time


# async def send_request(client, semaphore, i):
#     async with semaphore:
#         try:
#             # We open the file fresh for each request to avoid buffer issues
#             with open(FILE_PATH, "rb") as f:
#                 files = {"file": ("4.jpg", f, "image/jpeg")}
#                 response = await client.post(
#                     URL, headers=HEADERS, files=files, timeout=30.0
#                 )

#             if response.status_code == 200:
#                 print(f"[{i}] ✅ Success: {response.json()['job_id']}")
#             else:
#                 print(f"[{i}] ❌ Failed: {response.status_code} - {response.text}")
#         except Exception as e:
#             print(f"[{i}] 🔥 Error: {e}")


# async def main():
#     semaphore = asyncio.Semaphore(CONCURRENT_LIMIT)
#     async with httpx.AsyncClient() as client:
#         start_time = time.perf_counter()

#         tasks = [send_request(client, semaphore, i) for i in range(TOTAL_REQUESTS)]
#         await asyncio.gather(*tasks)

#         end_time = time.perf_counter()
#         total_time = end_time - start_time
#         print(f"\n{'=' * 30}")
#         print(f"STRESS TEST COMPLETE")
#         print(f"Total Requests: {TOTAL_REQUESTS}")
#         print(f"Total Time: {total_time:.2f} seconds")
#         print(f"Requests Per Second: {TOTAL_REQUESTS / total_time:.2f}")
#         print(f"{'=' * 30}")


# if __name__ == "__main__":
#     asyncio.run(main())
