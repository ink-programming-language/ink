// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(int)n;++i)");
}

func FOR(i: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();++i)");
}

func ALL(c: dynamic)
{
  return cpp_expression("#include <iostream> #i");
}

var INF = (1 << 29);

class Edge
{
  var src: dynamic;
  var dst: dynamic;
  var weight: dynamic;
  func Edge(src: dynamic, dst: dynamic, weight: dynamic)
  {
      this->src = cpp_construct(src);
      this->dst = cpp_construct(dst);
      this->weight = cpp_construct(weight);
    }
}

func operator_less(e: dynamic, f: dynamic)
{
  return if ((e.weight != f.weight)) (e.weight > f.weight) else if ((e.src != f.src)) (e.src < f.src) else (e.dst < f.dst);
}

func prim(g: dynamic, r: dynamic)
{
  var n = g.size();
  var T: dynamic;
  var total = 0;
  var Q: dynamic;
  REP(i, r).push(Edge(-1, i, 0));
  while ((!Q.empty()))
  {
    var e = Q.top();
    Q.pop();
    if (visited[e.dst])
    {
      continue;
    }
    T.push_back(e);
    total += e.weight;
    visited[e.dst] = true;
    FOR(f, g[e.dst]);
    if ((!visited[f->dst]))
    {
      Q.push((*f));
    }
  }
  return pair(total, T);
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  while (cpp_comma(((cin >> n) >> m), (n || m)))
  {
    var g = cpp_construct((n + m));
    write(prim(g, n).first, "\n");
  }
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
        var a: dynamic;
        read(a);
        if (a)
        {
          g[i].push_back(Edge(i, (j + n), a));
          g[(j + n)].push_back(Edge((j + n), i, a));
        }
      }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      {
        var j = (i + 1);
        while ((j < m))
        {
          var a: dynamic;
          read(a);
          if (a)
          {
            g[(i + n)].push_back(Edge((i + n), (j + n), a));
            g[(j + n)].push_back(Edge((j + n), (i + n), a));
          }
          j += 1;
        }
      }
    }
