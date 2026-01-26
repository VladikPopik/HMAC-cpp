import argparse
import asyncio
import random
import string
import typing as ty

import httpx
from tqdm import tqdm


def generate_random_msg():
    r_str = "".join(
        random.choices(string.ascii_uppercase + string.ascii_lowercase, k=16)
    )
    return r_str


async def run_(max_try_load: int, number_load_sim_req: int) -> None:

    with tqdm(
        total=3 * max_try_load, colour="green", position=0, leave=True
    ) as pbar:

        for _ in range(max_try_load):
            random_strs = [generate_random_msg() for _ in range(number_load_sim_req)]

            async with httpx.AsyncClient() as client:
                tmp = [
                    asyncio.create_task(
                        client.get("http://localhost:8080/ping")
                    )
                    for _ in range(number_load_sim_req)
                ]

                ping = await asyncio.gather(*tmp)
                ping_body = [r.json() for r in ping]
                assert all([p["status"] for p in ping_body])

            pbar.update()

            async with httpx.AsyncClient() as client:
                tmp = [
                    asyncio.create_task(
                        client.post(
                            "http://localhost:8080/sign",
                            json={"msg": random_strs[i]},
                        )
                    )
                    for i in range(number_load_sim_req)
                ]

                sign = await asyncio.gather(*tmp)

                sign_body = [r.json() for r in sign]

            pbar.update()

            async with httpx.AsyncClient() as client:
                tmp = [
                    asyncio.create_task(
                        client.post(
                            "http://localhost:8080/verify",
                            json={
                                "msg": random_strs[i],
                                "signature": sign_body[i]["signature"],
                            },
                        )
                    )
                    for i in range(100)
                ]

                verify = await asyncio.gather(*tmp)

                verify_body = [r.json() for r in verify]

                res = [value["ok"] for value in verify_body]

                assert all(res)

            pbar.update()


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description="A script that processes an integer.", add_help=False
    )
    parser.add_argument("max_try_load", type=int, help="Number of cycles load")
    parser.add_argument(
        "number_load_sim_req",
        type=int,
        help="Number of simultaneous requests per load",
    )

    parser.add_argument(
        "-h", "--help", "-help", action="help", help="Help instructions"
    )

    args = parser.parse_args()

    asyncio.run(run_(args.max_try_load, args.number_load_sim_req))
