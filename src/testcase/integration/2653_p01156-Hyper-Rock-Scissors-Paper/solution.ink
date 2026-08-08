// Translated from solution.cpp.

func FOR(i: dynamic, k: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(k); i<(int)n; ++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

func FORIT(i: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();++i)");
}

func debug(begin: dynamic, end: dynamic)
{
  {
    var i = begin;
    while ((i != end))
    {
      write((*i), " ");
      i += 1;
    }
  }
  write("\n");
}

var INF = 100000000;

var EPS = 1e-8;

var MOD = 1000000007;

func main()
{
  var hand = ["Rock", "Fire", "Scissors", "Snake", "Human", "Tree", "Wolf", "Sponge", "Paper", "Air", "Water", "Dragon", "Devil", "Lightning", "Gun"];
  var hand_id: dynamic;
  REP(i, 15)[hand[i]] = i;
  var N: dynamic;
  while (((cin >> N) && N))
  {
    REP(i, N);
    read(result[i]);
    REP(i, N);
    FOR(j, (i + 1), N);
    {
      var id1 = hand_id[result[i]];
      var id2 = hand_id[result[j]];
      if ((id1 == id2))
      {
      } else if ((((((id1 - id2) + 15)) % 15) <= 7))
      {
        win[j] += 1;
        lose[i] += 1;
      } else
      {
        assert((((((id2 - id1) + 15)) % 15) <= 7));
        win[i] += 1;
        lose[j] += 1;
      }
    }
    var winner = "Draw";
    REP(i, N);
    if (((win[i] > 0) && (lose[i] == 0)))
    {
      winner = result[i];
    }
    write(winner, "\n");
  }
  return 0;
}
