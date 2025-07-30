from dataclasses import dataclass


@dataclass
class Atom:
    index: int
    symbol: str
    x: float
    y: float
    z: float

    def move(self, x: float, y: float, z: float) -> None:
        self.x = x
        self.y = y
        self.z = z
