// Translated from solution.cpp.

var N = 100003;

func rd()
{
  var ch = getchar();
  var x = 0;
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

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    return cpp_comma(cpp_assign(a, "=", b), 1);
  }
  return 0;
}

var n: dynamic;

var G = cpp_array(N);

var V = cpp_array(N);

class DS
{
  var ps: dynamic;
  var ng: dynamic;
  var tag: dynamic;
  var pq: dynamic;
  func reb(x: dynamic, op: dynamic)
  {
      if (op)
      {
        var it = ng.upper_bound((x - ((tag << 1))));
        if ((it == ng.begin()))
        {
          return;
        }
        it -= 1;
        pq.emplace((x - it->first), x);
      } else
      {
        var it = ps.lower_bound((x + ((tag << 1))));
        if ((it == ps.end()))
        {
          return;
        }
        pq.emplace((it->first - x), it->first);
      }
    }
  func ins(x: dynamic, val: dynamic)
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
  func get()
  {
      var res = 0;
      var now = 0;
      var i = ps.begin();
      var j = ng.begin();
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
  func work(l: dynamic)
  {
      while ((!pq.empty()))
      {
        var cpp_name = pq.top();
        if ((cpp_name.first > (((tag + l) << 1))))
        {
          break;
        }
        pq.pop();
        var x = cpp_name.second;
        var y = (x - cpp_name.first);
        var i = ps.find(x);
        var j = ng.find(y);
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
  func size()
  {
      return (ps.size() + ng.size());
    }
  func qry(p: dynamic)
  {
      var i = ps.find((p + tag));
      var j = ng.find((p - tag));
      return ((if ((i == ps.end())) 0 else i->second) + (if ((j == ng.end())) 0 else j->second));
    }
  func operator_add_assign(o: dynamic)
  {
      for (var i in o.ps)
      {
        ins((i.first - o.tag), i.second);
      }
      for (var i in o.ng)
      {
        ins((i.first + o.tag), i.second);
      }
    }
}

var S = cpp_array(N);

func dfs(x: dynamic, fa: dynamic)
{
  for (var cpp_name in G[x])
  {
    if ((cpp_name.first != fa))
    {
      var v = cpp_name.first;
      var len = cpp_name.second;
      dfs(v, x);
      for (var i in V[v])
      {
        i.second -= max(0, max((-S[v].qry(i.first)), S[v].qry((i.first + 1))));
      }
      S[v].work(1);
      for (var i in V[v])
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

func main()
{
  n = rd();
  {
    var i = 1;
    while ((i < n))
    {
      var u = rd();
      var v = rd();
      var l = (rd() << 1);
      G[u].emplace_back(v, l);
      G[v].emplace_back(u, l);
      i += 1;
    }
  }
  var m = rd();
  G[0].emplace_back(1, 2);
  while (cpp_update(m, "--"))
  {
    var d = (rd() << 1);
    var f = rd();
    var p = rd();
    V[p].emplace_back(d, f);
  }
  dfs(0, 0);
  printf("%d\n", S[0].get());
}
