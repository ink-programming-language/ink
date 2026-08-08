// Translated from solution.cpp.

func P()
{
  return cpp_expression("#include <bits/stdc++.h>");
}

func FOR(x: dynamic, to: dynamic)
{
  cpp_macro("for(x=0;x<(to);x++)");
}

func FORR(x: dynamic, arr: dynamic)
{
  return cpp_expression("#include <bits/s");
}

func ITR(x: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof(c.begin()) x=c.begin();x!=c.end();x++)");
}

func ALL(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func ZERO(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func MINUS(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

var N: dynamic;

var M: dynamic;

var A = cpp_array(404040);

var B = cpp_array(404040);

var C = cpp_array(404040);

var E = cpp_array(4040);

var Q: dynamic;

var D = cpp_array(4040, 4040);

class UF
{
  var par: dynamic;
  var rank: dynamic;
  func UF()
  {
      rank = vector(um, 0);
      {
        var i = 0;
        while ((i < um))
        {
          par.push_back(i);
          i += 1;
        }
      }
    }
  func operator_index(x: dynamic)
  {
      return if (((par[x] == x))) (x) else (cpp_assign(par[x], "=", operator(par[x])));
    }
  func operator_call(x: dynamic, y: dynamic)
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

var uf: dynamic;

func dfs(st: dynamic, cur: dynamic, pre: dynamic, ma: dynamic)
{
  D[st][cur] = ma;
  FORR(e, E[cur]);
  if ((e.first != pre))
  {
    dfs(st, e.first, cur, max(ma, e.second));
  }
}

func solve()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  var r: dynamic;
  var x: dynamic;
  var y: dynamic;
  var s: dynamic;
  read(N, M);
  var EE: dynamic;
  var tot = 0;
  while (EE.size())
  {
    var e = EE.top();
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

func main(argc: dynamic, argv: dynamic)
{
  var s: dynamic;
  var i: dynamic;
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

func FOR(argument_0: dynamic, argument_1: dynamic)
{
    read(A[i], B[i], C[i]);
    A[i] -= 1;
    B[i] -= 1;
    EE.push([(-C[i]), i]);
  }

func FOR(argument_0: dynamic, argument_1: dynamic)
{
    read(x, y);
    write((tot - D[(x - 1)][(y - 1)]), "\n");
  }
