// Translated from solution.cpp.

var ll: dynamic = dynamic;

var INF: dynamic = cpp_expression("#include <");

var MOD: dynamic = cpp_expression("#include <");

var EPS: dynamic = cpp_expression("#incl");

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)(n);++i)");
}

func rrep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(int)(n)-1;i>=0;--i)");
}

func srep(i: dynamic, s: dynamic, t: dynamic) -> dynamic
{
  cpp_macro("for(int i=(int)(s);i<(int)(t);++i)");
}

func each(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc");
}

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

func len(v: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

func zip(v: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> #define ll long lon");
}

func cmx(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

func cmn(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var pb: dynamic = cpp_expression("#include");

func show(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> #");
}

func spair(p: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> #define ll l");
}

func sar(a: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("cout<<#a<<\":\";rep(pachico,n)cout<<\" \"<<a[pachico];cout<<endl");
}

func svec(v: dynamic) -> dynamic
{
  cpp_macro("cout<<#v<<\":\";rep(pachico,v.size())cout<<\" \"<<v[pachico];cout<<endl");
}

func svecp(v: dynamic) -> dynamic
{
  cpp_macro("cout<<#v<<\":\";each(pachico,v)cout<<\" {\"<<pachico.first<<\":\"<<pachico.second<<\"}\";cout<<endl");
}

func sset(s: dynamic) -> dynamic
{
  cpp_macro("cout<<#s<<\":\";each(pachico,s)cout<<\" \"<<pachico;cout<<endl");
}

func smap(m: dynamic) -> dynamic
{
  cpp_macro("cout<<#m<<\":\";each(pachico,m)cout<<\" {\"<<pachico.first<<\":\"<<pachico.second<<\"}\";cout<<endl");
}

var MAX_N: dynamic = 2005;

var G: dynamic = cpp_array(MAX_N);

var dp: dynamic = cpp_array(2, 3, MAX_N, MAX_N);

var st: dynamic = cpp_array(MAX_N);

var fac: dynamic = cpp_array(MAX_N);

var inv2: dynamic = cpp_uninitialized();

func make() -> dynamic
{
  fac[0] = cpp_assign(fac[1], "=", 1);
  {
    var i: dynamic = 2;
    while ((i < MAX_N))
    {
      fac[i] = ((cpp_cast(fac[(i - 1)]) * i) % MOD);
      i += 1;
    }
  }
}

func mod_pow(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while (b)
  {
    if ((b & 1))
    {
      res = ((cpp_cast(res) * a) % MOD);
    }
    a = ((cpp_cast(a) * a) % MOD);
    b >>= 1;
  }
  return res;
}

func add(x: dynamic, y: dynamic) -> dynamic
{
  return (((x + y)) % MOD);
}

func sub(x: dynamic, y: dynamic) -> dynamic
{
  return ((((x + MOD) - y)) % MOD);
}

func mul(x: dynamic, y: dynamic) -> dynamic
{
  return ((cpp_cast(x) * y) % MOD);
}

func dfs(u: dynamic) -> dynamic
{
  st[u] = 1;
  dp[u][1][2][0] = 1;
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  read(n);
  rep(i, (n - 1));
  {
    var p: dynamic = cpp_uninitialized();
    read(p);
    G[(p - 1)].pb((i + 1));
  }
  make();
  inv2 = mod_pow(2, (MOD - 2));
  dfs(0);
  var ans: dynamic = 0;
  srep(i, 1, (n + 1));
  {
    if ((((n - i)) % 2))
    {
      ans = sub(ans, mul(((((cpp_cast(dp[0][i][0][0]) + dp[0][i][1][0]) + dp[0][i][2][0])) % MOD), fac[i]));
    } else
    {
      ans = add(ans, mul(((((cpp_cast(dp[0][i][0][0]) + dp[0][i][1][0]) + dp[0][i][2][0])) % MOD), fac[i]));
    }
  }
  write(ans, "\n");
  return 0;
}

func each(argument_0: dynamic, u: dynamic) -> dynamic
{
    dfs(v);
    {
      var i: dynamic = st[u];
      while ((i >= 1))
      {
        {
          var j: dynamic = st[v];
          while ((j >= 1))
          {
            dp[u][(i + j)][2][1] = add(dp[u][(i + j)][2][1], mul(dp[u][i][2][0], ((((cpp_cast(dp[v][j][0][0]) + dp[v][j][1][0]) + dp[v][j][2][0])) % MOD)));
            dp[u][(i + j)][1][1] = add(dp[u][(i + j)][1][1], mul(dp[u][i][1][0], ((((cpp_cast(dp[v][j][0][0]) + dp[v][j][1][0]) + dp[v][j][2][0])) % MOD)));
            dp[u][(i + j)][0][1] = add(dp[u][(i + j)][0][1], mul(dp[u][i][0][0], ((((cpp_cast(dp[v][j][0][0]) + dp[v][j][1][0]) + dp[v][j][2][0])) % MOD)));
            dp[u][((i + j) - 1)][1][1] = add(dp[u][((i + j) - 1)][1][1], mul(mul(dp[u][i][2][0], dp[v][j][2][0]), 2));
            dp[u][((i + j) - 1)][1][1] = add(dp[u][((i + j) - 1)][1][1], mul(dp[u][i][2][0], dp[v][j][1][0]));
            dp[u][((i + j) - 1)][0][1] = add(dp[u][((i + j) - 1)][0][1], mul(dp[u][i][1][0], dp[v][j][2][0]));
            dp[u][((i + j) - 1)][0][1] = add(dp[u][((i + j) - 1)][0][1], mul(mul(dp[u][i][1][0], dp[v][j][1][0]), inv2));
            j -= 1;
          }
        }
        i -= 1;
      }
    }
    st[u] += st[v];
    srep(i, 1, (st[u] + 1));
    {
      cpp_statement("rep(j,3)");
      {
        dp[u][i][j][0] = dp[u][i][j][1];
        dp[u][i][j][1] = 0;
      }
    }
  }
