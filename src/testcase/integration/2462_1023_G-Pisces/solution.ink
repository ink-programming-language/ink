// Translated from solution.cpp.

var N: dynamic = 100003;

func rd() -> dynamic
{
  var ch: dynamic = getchar();
  var x: dynamic = 0;
  {
    while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
    {
      ch = getchar();
    }
  }
  {
    while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
    {
      x = (((x * 10) + ch) - cpp_char("0"));
      ch = getchar();
    }
  }
  return x;
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    return cpp_comma(cpp_assign(a, "=", b), 1);
  }
  return 0;
}

var n: dynamic = cpp_uninitialized();

var G: dynamic = cpp_array(N);

var V: dynamic = cpp_array(N);

class DS
{
  var ps: dynamic = cpp_uninitialized();
  var ng: dynamic = cpp_uninitialized();
  var tag: dynamic = cpp_uninitialized();
  var pq: dynamic = cpp_uninitialized();
  func reb(x: dynamic, op: dynamic) -> dynamic
  {
      if (op)
      {
        var it: dynamic = ng.upper_bound((x - ((tag << 1))));
        if ((it == ng.begin()))
        {
          return;
        }
        it -= 1;
        pq.emplace((x - it->first), x);
      } else
      {
        var it: dynamic = ps.lower_bound((x + ((tag << 1))));
        if ((it == ps.end()))
        {
          return;
        }
        pq.emplace((it->first - x), it->first);
      }
    }
  func ins(x: dynamic, val: dynamic) -> dynamic
  {
      if ((val > 0))
      {
        ps[(x + tag)] += val;
        reb((x + tag), true);
      } else
      {
        ng[(x - tag)] += val;
        reb((x - tag), false);
      }
    }
  func get() -> dynamic
  {
      var res: dynamic = 0;
      var now: dynamic = 0;
      var i: dynamic = ps.begin();
      var j: dynamic = ng.begin();
      while (((i != ps.end()) && (j != ng.end())))
      {
        if (((i->first - j->first) >= ((tag << 1))))
        {
          now += j->second;
          j += 1;
        } else
        {
          now += i->second;
          i += 1;
        }
        chmax(res, now);
      }
      return res;
    }
  func work(l: dynamic) -> dynamic
  {
      while ((!pq.empty()))
      {
        var cpp_name: dynamic = pq.top();
        if ((cpp_name.first > (((tag + l) << 1))))
        {
          break;
        }
        pq.pop();
        var x: dynamic = cpp_name.second;
        var y: dynamic = (x - cpp_name.first);
        var i: dynamic = ps.find(x);
        var j: dynamic = ng.find(y);
        if (((i == ps.end()) || (j == ng.end())))
        {
          continue;
        }
        if (((i->second + j->second) < 0))
        {
          j->second += i->second;
          ps.erase(i);
          reb(j->first, false);
        } else
        {
          i->second += j->second;
          ng.erase(j);
          reb(i->first, true);
        }
      }
      tag += l;
    }
  func size() -> dynamic
  {
      return (ps.size() + ng.size());
    }
  func qry(p: dynamic) -> dynamic
  {
      var i: dynamic = ps.find((p + tag));
      var j: dynamic = ng.find((p - tag));
      return (( ((i == ps.end())) ? 0 : i->second) + ( ((j == ng.end())) ? 0 : j->second));
    }
  func operator_add_assign(o: dynamic) -> dynamic
  {
      for (var i: dynamic in o.ps)
      {
        ins((i.first - o.tag), i.second);
      }
      for (var i: dynamic in o.ng)
      {
        ins((i.first + o.tag), i.second);
      }
    }
}

var S: dynamic = cpp_array(N);

func dfs(x: dynamic, fa: dynamic) -> dynamic
{
  for (var cpp_name: dynamic in G[x])
  {
    if ((cpp_name.first != fa))
    {
      var v: dynamic = cpp_name.first;
      var len: dynamic = cpp_name.second;
      dfs(v, x);
      for (var i: dynamic in V[v])
      {
        i.second -= max(0, max((-S[v].qry(i.first)), S[v].qry((i.first + 1))));
      }
      S[v].work(1);
      for (var i: dynamic in V[v])
      {
        if ((i.second > 0))
        {
          S[v].ins(i.first, i.second);
          S[v].ins((i.first + 1), (-i.second));
        }
      }
      S[v].work((len - 1));
      if ((S[x].size() < S[v].size()))
      {
        swap(S[x], S[v]);
      }
      S[x] += S[v];
    }
  }
}

func main() -> dynamic
{
  n = rd();
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = rd();
      var v: dynamic = rd();
      var l: dynamic = (rd() << 1);
      G[u].emplace_back(v, l);
      G[v].emplace_back(u, l);
      i += 1;
    }
  }
  var m: dynamic = rd();
  G[0].emplace_back(1, 2);
  while (cpp_update(m, "--"))
  {
    var d: dynamic = (rd() << 1);
    var f: dynamic = rd();
    var p: dynamic = rd();
    V[p].emplace_back(d, f);
  }
  dfs(0, 0);
  printf("%d\n", S[0].get());
}
