// Translated from solution.cpp.

func FOR(i: dynamic, k: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(k); i<(int)(n); ++i)");
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

func valid(x: dynamic, y: dynamic, W: dynamic, H: dynamic)
{
  return (((((x >= 0) && (y >= 0)) && (x < W)) && (y < H)));
}

var INF = 100000000;

var EPS = 1e-8;

var MOD = 1000000007;

var dx = [1, 0, -1, 0, 1, -1, -1, 1];

var dy = [0, 1, 0, -1, 1, 1, -1, -1];

class S
{
  var u: dynamic;
  var cost: dynamic;
  var change: dynamic;
  func S()
  {
    }
  func S(u: dynamic, a: dynamic, b: dynamic)
  {
      this->u = cpp_construct(u);
      this->cost = cpp_construct(a);
      this->change = cpp_construct(b);
    }
  func operator_less(s: dynamic)
  {
      if ((cost != s.cost))
      {
        return (cost > s.cost);
      }
      return (change > s.change);
    }
}

var MAX_V = 100000;

func main()
{
  var N: dynamic;
  var T: dynamic;
  while (((cin >> N) >> T))
  {
    var ST: dynamic;
    var GL: dynamic;
    read(ST, GL);
    var st_ids: dynamic;
    var to = cpp_array(MAX_V);
    var cost = cpp_array(MAX_V);
    var name = cpp_array(MAX_V);
    var V = 0;
    var que: dynamic;
    var used: dynamic;
    var used2 = [];
    used.insert(ST);
    REP(i, st_ids[ST].size());
    {
      que.push(S(st_ids[ST][i], 0, 0));
    }
    var ok = false;
    while ((!que.empty()))
    {
      var s = que.top();
      que.pop();
      if ((name[s.u] == GL))
      {
        printf("%d %d\n", s.cost, s.change);
        ok = true;
        break;
      }
      if (used2[s.u])
      {
        continue;
      }
      used2[s.u] = true;
      if ((!used.count(name[s.u])))
      {
        REP(i, st_ids[name[s.u]].size());
        {
          que.push(S(st_ids[name[s.u]][i], (s.cost + T), (s.change + 1)));
        }
        used.insert(name[s.u]);
      }
      REP(i, to[s.u].size());
      {
        var v = to[s.u][i];
        var c = cost[s.u][i];
        que.push(S(v, (s.cost + c), s.change));
      }
    }
    if ((!ok))
    {
      write(-1, "\n");
    }
  }
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var A: dynamic;
      read(A);
      REP(i, A);
      read(name[(V + i)]);
      REP(i, A)[name[(V + i)]].push_back((V + i));
      REP(i, (A - 1));
      {
        var t: dynamic;
        read(t);
        to[(V + i)].push_back(((V + i) + 1));
        cost[(V + i)].push_back(t);
        to[((V + i) + 1)].push_back((V + i));
        cost[((V + i) + 1)].push_back(t);
      }
      V += A;
    }
