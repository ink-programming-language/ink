// Translated from solution.cpp.

func P() -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h>");
}

func FOR(x: dynamic, to: dynamic) -> dynamic
{
  cpp_macro("for(x=0;x<(to);x++)");
}

func FORR(x: dynamic, arr: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/s");
}

func ITR(x: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof(c.begin()) x=c.begin();x!=c.end();x++)");
}

func ALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

func ZERO(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

func MINUS(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h>");
}

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(404040);

var B: dynamic = cpp_array(404040);

var C: dynamic = cpp_array(404040);

var E: dynamic = cpp_array(4040);

var Q: dynamic = cpp_uninitialized();

var D: dynamic = cpp_array(4040, 4040);

class UF
{
  var par: dynamic = cpp_uninitialized();
  var rank: dynamic = cpp_uninitialized();
  func UF() -> dynamic
  {
      rank = vector(um, 0);
      {
        var i: dynamic = 0;
        while ((i < um))
        {
          par.push_back(i);
          i += 1;
        }
      }
    }
  func operator_index(x: dynamic) -> dynamic
  {
      return  (((par[x] == x))) ? (x) : (cpp_assign(par[x], "=", operator(par[x])));
    }
  func operator_call(x: dynamic, y: dynamic) -> dynamic
  {
      if (((cpp_assign(x, "=", operator(x))) == (cpp_assign(y, "=", operator(y)))))
      {
        return x;
      }
      if ((rank[x] > rank[y]))
      {
        return cpp_assign(par[x], "=", y);
      }
      rank[x] += (rank[x] == rank[y]);
      return cpp_assign(par[y], "=", x);
    }
}

var uf: dynamic = cpp_uninitialized();

func dfs(st: dynamic, cur: dynamic, pre: dynamic, ma: dynamic) -> dynamic
{
  D[st][cur] = ma;
  FORR(e, E[cur]);
  if ((e.first != pre))
  {
    dfs(st, e.first, cur, max(ma, e.second));
  }
}

func solve() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(N, M);
  var EE: dynamic = cpp_uninitialized();
  var tot: dynamic = 0;
  while (EE.size())
  {
    var e: dynamic = EE.top();
    x = A[e.second];
    y = B[e.second];
    EE.pop();
    if ((uf[x] != uf[y]))
    {
      uf(x, y);
      E[x].push_back([y, (-e.first)]);
      E[y].push_back([x, (-e.first)]);
      tot += (-e.first);
    }
  }
  FOR(i, N);
  dfs(i, i, -1, 0);
  read(M);
}

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  if ((argc == 1))
  {
    ios.sync_with_stdio(false);
    cin.tie(0);
  }
  FOR(i, (argc - 1)) += argv[(i + 1)];
  s += cpp_char("\n");
  FOR(i, s.size());
  ungetc(s[((s.size() - 1) - i)], stdin);
  solve();
  return 0;
}

func FOR(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    read(A[i], B[i], C[i]);
    A[i] -= 1;
    B[i] -= 1;
    EE.push([(-C[i]), i]);
  }

func FOR(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    read(x, y);
    write((tot - D[(x - 1)][(y - 1)]), "\n");
  }
