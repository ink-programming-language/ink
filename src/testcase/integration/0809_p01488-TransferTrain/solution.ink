// Translated from solution.cpp.

func FOR(i: dynamic, k: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(k); i<(int)(n); ++i)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

func FORIT(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();++i)");
}

func debug(begin: dynamic, end: dynamic) -> dynamic
{
  {
    var i: dynamic = begin;
    while ((i != end))
    {
      write((*i), " ");
      i += 1;
    }
  }
  write("\n");
}

func valid(x: dynamic, y: dynamic, W: dynamic, H: dynamic) -> dynamic
{
  return (((((x >= 0) && (y >= 0)) && (x < W)) && (y < H)));
}

var INF: dynamic = 100000000;

var EPS: dynamic = 1e-8;

var MOD: dynamic = 1000000007;

var dx: dynamic = [1, 0, -1, 0, 1, -1, -1, 1];

var dy: dynamic = [0, 1, 0, -1, 1, 1, -1, -1];

class S
{
  var u: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  var change: dynamic = cpp_uninitialized();
  func S() -> dynamic
  {
    }
  func S(u: dynamic, a: dynamic, b: dynamic) -> dynamic
  {
      self->u = cpp_construct(u);
      self->cost = cpp_construct(a);
      self->change = cpp_construct(b);
    }
  func operator_less(s: dynamic) -> dynamic
  {
      if ((cost != s.cost))
      {
        return (cost > s.cost);
      }
      return (change > s.change);
    }
}

var MAX_V: dynamic = 100000;

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var T: dynamic = cpp_uninitialized();
  while (((cin >> N) >> T))
  {
    var ST: dynamic = cpp_uninitialized();
    var GL: dynamic = cpp_uninitialized();
    read(ST, GL);
    var st_ids: dynamic = cpp_uninitialized();
    var to: dynamic = cpp_array(MAX_V);
    var cost: dynamic = cpp_array(MAX_V);
    var name: dynamic = cpp_array(MAX_V);
    var V: dynamic = 0;
    var que: dynamic = cpp_uninitialized();
    var used: dynamic = cpp_uninitialized();
    var used2: dynamic = [];
    used.insert(ST);
    REP(i, st_ids[ST].size());
    {
      que.push(S(st_ids[ST][i], 0, 0));
    }
    var ok: dynamic = false;
    while ((!que.empty()))
    {
      var s: dynamic = que.top();
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
        var v: dynamic = to[s.u][i];
        var c: dynamic = cost[s.u][i];
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

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var A: dynamic = cpp_uninitialized();
      read(A);
      REP(i, A);
      read(name[(V + i)]);
      REP(i, A)[name[(V + i)]].push_back((V + i));
      REP(i, (A - 1));
      {
        var t: dynamic = cpp_uninitialized();
        read(t);
        to[(V + i)].push_back(((V + i) + 1));
        cost[(V + i)].push_back(t);
        to[((V + i) + 1)].push_back((V + i));
        cost[((V + i) + 1)].push_back(t);
      }
      V += A;
    }
