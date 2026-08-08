// Translated from solution.cpp.

func rep(i: dynamic, j: dynamic)
{
  cpp_macro("for(int__ i=0;i<(int__)(j);i++)");
}

func repeat(i: dynamic, j: dynamic, k: dynamic)
{
  cpp_macro("for(int__ i=(j);i<(int__)(k);i++)");
}

func all(v: dynamic)
{
  return cpp_expression("#include<iostream");
}

class UnionFind
{
  var n: dynamic;
  var p: dynamic;
  func UnionFind(nn: dynamic)
  {
      this->n = cpp_construct((nn + 1));
      p.resize(n);
      rep(i, n)[i] = i;
    }
  func root(x: dynamic)
  {
      if ((p[x] == x))
      {
        return x;
      } else
      {
        return cpp_assign(p[x], "=", root(p[x]));
      }
    }
  func unite(x: dynamic, y: dynamic)
  {
      x = root(x);
      y = root(y);
      if ((x != y))
      {
        p[y] = x;
      }
    }
  func query(x: dynamic, y: dynamic)
  {
      return (root(x) == root(y));
    }
}

func main()
{
  ios.sync_with_stdio(false);
  var N: dynamic;
  var M: dynamic;
  read(N, M);
  var c: dynamic;
  var s: dynamic;
  var city = s.size();
  var vil = (N - c.size());
  write(abs((vil - city)), "\n");
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var a: dynamic;
    var b: dynamic;
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var r = ut.root(i);
    if ((r != i))
    {
      s.insert(ut.root(i));
    }
  }
