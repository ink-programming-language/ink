// Translated from solution.cpp.

func rep(i: dynamic, j: dynamic) -> dynamic
{
  cpp_macro("for(int__ i=0;i<(int__)(j);i++)");
}

func repeat(i: dynamic, j: dynamic, k: dynamic) -> dynamic
{
  cpp_macro("for(int__ i=(j);i<(int__)(k);i++)");
}

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream");
}

class UnionFind
{
  var n: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  func UnionFind(nn: dynamic) -> dynamic
  {
      self->n = cpp_construct((nn + 1));
      p.resize(n);
      rep(i, n)[i] = i;
    }
  func root(x: dynamic) -> dynamic
  {
      if ((p[x] == x))
      {
        return x;
      } else
      {
        return cpp_assign(p[x], "=", root(p[x]));
      }
    }
  func unite(x: dynamic, y: dynamic) -> dynamic
  {
      x = root(x);
      y = root(y);
      if ((x != y))
      {
        p[y] = x;
      }
    }
  func query(x: dynamic, y: dynamic) -> dynamic
  {
      return (root(x) == root(y));
    }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  read(N, M);
  var c: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var city: dynamic = s.size();
  var vil: dynamic = (N - c.size());
  write(abs((vil - city)), "\n");
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(a, b);
    a -= 1;
    b -= 1;
    if ((a != b))
    {
      c.insert(a);
      c.insert(b);
    }
    ut.unite(a, b);
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var r: dynamic = ut.root(i);
    if ((r != i))
    {
      s.insert(ut.root(i));
    }
  }
