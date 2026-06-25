#include "../../include/types.h"
#include "../../include/bag.h"
#include "../../include/battle.h"
#include "../../include/config.h"
#include "../../include/debug.h"
#include "../../include/message.h"
#include "../../include/party_menu.h"
#include "../../include/pokemon.h"
#include "../../include/rtc.h"
#include "../../include/save.h"
#include "../../include/script.h"
#include "../../include/window.h"
#include "../../include/constants/ability.h"
#include "../../include/constants/file.h"
#include "../../include/constants/game.h"
#include "../../include/constants/hold_item_effects.h"
#include "../../include/constants/item.h"
#include "../../include/constants/moves.h"
#include "../../include/constants/species.h"
#include "../../include/constants/weather_numbers.h"
#include "../../include/constants/generated/learnsets.h"

#define MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     0
#define MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    1
#define MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT 2
#define MOVE_TUTOR_NPC_HEADBUTT              3

typedef struct TutorMove {
    u16 move;
    u8 cost;
    u8 tutorNpc;
} TutorMove;

TutorMove sTutorMoves[] = {
    { MOVE_DIVE,          0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_MUD_SLAP,      0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_FURY_CUTTER,   0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_ICY_WIND,      0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_ROLLOUT,       0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_THUNDER_PUNCH, 0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_FIRE_PUNCH,    0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_SUPERPOWER,    0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_ICE_PUNCH,     0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_IRON_HEAD,     0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_AQUA_TAIL,     0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_OMINOUS_WIND,  0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_GASTRO_ACID,   0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_SNORE,         0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_SPITE,         0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_AIR_CUTTER,    0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_HELPING_HAND,  0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_ENDEAVOR,      0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_OUTRAGE,       0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_ANCIENT_POWER, 0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_SYNTHESIS,     0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_SIGNAL_BEAM,   0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_ZEN_HEADBUTT,  0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_VACUUM_WAVE,   0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_EARTH_POWER,   0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_GUNK_SHOT,     0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_TWISTER,       0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_SEED_BOMB,     0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_IRON_DEFENSE,  0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_MAGNET_RISE,   0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_LAST_RESORT,   0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_BOUNCE,        0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_TRICK,         0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_HEAT_WAVE,     0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_KNOCK_OFF,     0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_SUCKER_PUNCH,  0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_SWIFT,         0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_UPROAR,        0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_SUPER_FANG,    0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_PAIN_SPLIT,    0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_STRING_SHOT,   0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_TAILWIND,      0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_GRAVITY,       0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_WORRY_SEED,    0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_MAGIC_COAT,    0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_ROLE_PLAY,     0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_HEAL_BELL,     0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_LOW_KICK,      0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_SKY_ATTACK,    0,  MOVE_TUTOR_NPC_FRONTIER_TOP_RIGHT    },
    { MOVE_BLOCK,         0,  MOVE_TUTOR_NPC_FRONTIER_BOTTOM_RIGHT },
    { MOVE_BUG_BITE,      0,  MOVE_TUTOR_NPC_FRONTIER_TOP_LEFT     },
    { MOVE_HEADBUTT,      0,  MOVE_TUTOR_NPC_HEADBUTT              },
};

u16 LONG_CALL GetLearnableTutorMoves(struct PartyPokemon *mon, u32 moveTutorNpc, u8 dest[]) {
    int i;
    u16 currentMoves[MAX_MON_MOVES];
    for (i = 0; i < MAX_MON_MOVES; i++) {
        currentMoves[i] = GetMonData(mon, MON_DATA_MOVE1 + i, NULL);
    }

    u32 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u32 form = GetMonData(mon, MON_DATA_FORM, NULL);
    u32 tutorLearnset[TUTOR_LEARNSETS_BITFIELD_COUNT];

    ArchiveDataLoadOfs(tutorLearnset, ARC_CODE_ADDONS, CODE_ADDON_TUTOR_LEARNSETS, PokeOtherFormMonsNoGet(species, form) * TUTOR_LEARNSETS_BITFIELD_COUNT  * sizeof(u32), TUTOR_LEARNSETS_BITFIELD_COUNT  * sizeof(u32));

    u16 numLearnableMoves = 0;
    for (u32 j = 0; j < NELEMS(sTutorMoves); j++) {
        BOOL canLearnMove = (tutorLearnset[j / 32] >> (j % 32)) & 1;
        if (canLearnMove && moveTutorNpc == sTutorMoves[j].tutorNpc) {
            for (i = 0; i < MAX_MON_MOVES; i++) {
                if (currentMoves[i] == sTutorMoves[j].move) {
                    break;
                }
            }
            if (i < MAX_MON_MOVES) {
                continue;
            }
            if (dest != NULL) {
                dest[numLearnableMoves] = j;
            }
            numLearnableMoves++;
        }
    }

    return numLearnableMoves;
}
