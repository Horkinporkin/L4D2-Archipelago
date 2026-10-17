# "death" = player actually died / team wipe.
DEATH_CAUSE_MESSAGES: dict[str, dict[str, str]] = {
    "Hunter": {
        "downed": "was pinned to the ground by a Hunter",
        "death": "was cut open by a Hunter",
    },
    "Hunter Claw": {
        "downed": "was slashed by a Hunter's claws",
        "death": "was clawed to death by a Hunter",
    },
    "Smoker": {
        "downed": "was dragged off by a Smoker's tongue",
        "death": "was strangled by a Smoker's tongue",
    },
    "Smoker Claw": {
        "downed": "was shredded by a Smoker's claws",
        "death": "was clawed to death by a Smoker",
    },
    "Boomer": {
        "downed": "was caught in a Boomer's explosion",
        "death": "was blown apart by a Boomer's explosion",
    },
    "Boomer Claw": {
        "downed": "was scratched up by a Boomer",
        "death": "was clawed to death by a Boomer",
    },
    "Spitter": {
        "downed": "was melted by a Spitter's acid",
        "death": "was dissolved by a Spitter's acid",
    },
    "Spitter Claw": {
        "downed": "was clawed by a Spitter",
        "death": "was clawed to death by a Spitter",
    },
    "Jockey": {
        "downed": "was dragged around by a Jockey",
        "death": "was ridden into the ground by a Jockey",
    },
    "Jockey Claw": {
        "downed": "was scratched up by a Jockey",
        "death": "was clawed to death by a Jockey",
    },
    "Charger": {
        "downed": "was knocked down by a charging Charger",
        "death": "was pummeled to death by a Charger",
    },
    "Charger Claw": {
        "downed": "was mauled by a Charger's claws",
        "death": "was clawed to death by a Charger",
    },
    "Tank": {
        "downed": "was knocked down by a Tank",
        "death": "was beaten to death by a Tank",
    },
    "Tank Claw": {
        "downed": "was pummeled by a Tank's fists",
        "death": "was battered to death by a Tank's fists",
    },
    "Tank Rock": {
        "downed": "was crushed by a Tank's flying rock",
        "death": "was crushed to death by a Tank's rock",
    },
    "Witch": {
        "downed": "was mauled by a raging Witch",
        "death": "was torn apart by a Witch",
    },
    "Common": {
        "downed": "was knocked down by the horde",
        "death": "was overwhelmed by the horde",
    },
    "Fire": {
        "downed": "was engulfed in flames",
        "death": "was burned alive",
    },
    "Fall": {
        "downed": "plummeted to the ground",
        "death": "fell to their death",
    },
    "Water": {
        "downed": "drowned",
        "death": "drowned",
    },
    "World": {
        "downed": "died to the world",
        "death": "died to the world",
    },
    "Mission Lost": {
        "downed": "wiped out",
        "death": "wiped out",
    },
    "Unknown": {
        "downed": "went down for an unknown reason",
        "death": "died for an unknown reason",
    },
}


def format_death_message(base_cause: str, is_death: bool, player_name: str) -> str:
    cause = (base_cause or "").strip()
    message = DEATH_CAUSE_MESSAGES.get(cause, {}).get("death" if is_death else "downed")

    if message is None:
        if cause:
            message = f"{'died to' if is_death else 'went down to'} {cause}"
        else:
            message = "died" if is_death else "went down"

    return f"{player_name} {message}."
