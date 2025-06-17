#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
using namespace std;
using namespace __gnu_pbds;

struct Song { int id; long long suma; int votos; double media() const { return 1.0 * suma / votos; } };

struct Cmp {
    bool operator()(const Song* a, const Song* b) const {
        if (a->media() != b->media()) return a->media() > b->media();
        if (a->votos  != b->votos )  return a->votos  > b->votos;
        return a->id < b->id;
    }
};

using Ranking = tree<Song*, null_type, Cmp, rb_tree_tag, tree_order_statistics_node_update>;

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    if (argc != 2) return 1;
    ifstream in(argv[1]); if (!in) return 1;

    unordered_map<int, Song*> idx;
    Ranking ranking;
    string line;

    while (getline(in, line)) {
        const char* s = line.c_str();
        char* end;
        strtol(s, &end, 10);                    
        long songId = strtol(end + 1, &end, 10);
        long rating = strtol(end + 1, nullptr, 10);

        if (rating < 1 || rating > 5) continue;

        Song* song;
        auto it = idx.find(songId);
        if (it == idx.end()) {
            song = new Song{(int)songId, 0, 0};
            idx[songId] = song;
        } else {
            song = it->second;
            ranking.erase(song);
        }
        song->suma += rating;
        song->votos += 1;
        ranking.insert(song);
    }

    cout << "Introduce k: " << flush;
    int k; if (!(cin >> k)) return 0;

    cout << "\n=== Top " << k << " ===\n";
    int printed = 0;
    for (auto it = ranking.begin(); it != ranking.end() && printed < k; ++it, ++printed)
        cout << (*it)->id << " " << fixed << setprecision(3) << (*it)->media() << " " << (*it)->votos << '\n';

    cout << "\n=== Bottom " << k << " ===\n";
    printed = 0;
    for (auto it = ranking.rbegin(); it != ranking.rend() && printed < k; ++it, ++printed)
        cout << (*it)->id << " " << fixed << setprecision(3) << (*it)->media() << " " << (*it)->votos << '\n';
}
