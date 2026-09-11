// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)n;++i)");
}

func FOR(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();++i)");
}

func ALL(c: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #i");
}

var INF: dynamic = (1 << 29);

class Edge
{
  var src: dynamic = cpp_uninitialized();
  var dst: dynamic = cpp_uninitialized();
  var weight: dynamic = cpp_uninitialized();
  func Edge(src: dynamic, dst: dynamic, weight: dynamic) -> dynamic
  {
      self->src = cpp_construct(src);
      self->dst = cpp_construct(dst);
      self->weight = cpp_construct(weight);
    }
}

func operator_less(e: dynamic, f: dynamic) -> dynamic
{
  return  ((e.weight != f.weight)) ? (e.weight > f.weight) :  ((e.src != f.src)) ? (e.src < f.src) : (e.dst < f.dst);
}

func prim(g: dynamic, r: dynamic) -> dynamic
{
  var n: dynamic = g.size();
  var T: dynamic = cpp_uninitialized();
  var total: dynamic = 0;
  var Q: dynamic = cpp_uninitialized();
  REP(i, r).push(Edge(-1, i, 0));
  while ((!Q.empty()))
  {
    var e: dynamic = Q.top();
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  while (cpp_comma(((cin >> n) >> m), (n || m)))
  {
    var g: dynamic = cpp_construct((n + m));
    write(prim(g, n).first, "\n");
  }
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var a: dynamic = cpp_uninitialized();
        read(a);
        if (a)
        {
          g[i].push_back(Edge(i, (j + n), a));
          g[(j + n)].push_back(Edge((j + n), i, a));
        }
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      {
        var j: dynamic = (i + 1);
        while ((j < m))
        {
          var a: dynamic = cpp_uninitialized();
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
