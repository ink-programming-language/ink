// Translated from solution.cpp.

var MOD1 = (1e9 + 7);

var MOD2 = 998244353;

var INF = LLONG_MAX;

var PI = 3.14159265358979323846;

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func fpow(a: dynamic, b: dynamic, m: dynamic)
{
  if ((!b))
  {
    return 1;
  }
  var ans = fpow(((a * a) % m), (b / 2), m);
  return (if ((b % 2)) ((ans * a) % m) else ans);
}

func inv(a: dynamic, m: dynamic)
{
  return fpow(a, (m - 2), m);
}

var MottoHayaku = cpp_expression("#include <bits/stdc++.h> #in");

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(ll i=0;i<n;i++)");
}

func rep1(i: dynamic, n: dynamic)
{
  cpp_macro("for(ll i=1;i<=n;i++)");
}

func repk(i: dynamic, m: dynamic, n: dynamic)
{
  cpp_macro("for(int i=m;i<n;i++)");
}

var F = cpp_expression("#incl");

var S = cpp_expression("#inclu");

var pb = cpp_expression("#include");

func SZ(a: dynamic)
{
  return cpp_expression("#include <bits");
}

func reset(a: dynamic)
{
  return cpp_expression("#include <bits/st");
}

func cd(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <b");
}

func pow2(a: dynamic)
{
  return cpp_expression("#include");
}

func LB(a: dynamic, x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> #include");
}

func LPOS(a: dynamic, x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> #include <random> #");
}

func UB(a: dynamic, x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> #include");
}

func UPOS(a: dynamic, x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> #include <random> #");
}

func uni(c: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> #include <random> #include <ch");
}

func unisort(c: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> #incl");
}

var pos = cpp_array(300005);

var cnt = cpp_array(300005);

var s: dynamic;

var st: dynamic;

func main()
{
  var ll: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    rep1(i, n)[i].clear();
    cnt[i] = 0;
    st.insert(0);
    st.insert((n + 1));
    var x = 0;
    var ans: dynamic;
    var mn = INF;
    {
      var i = (ans.size() - 1);
      while ((i >= 0))
      {
        write(ans[i]);
        i -= 1;
      }
    }
    write("\n");
    st.clear();
    s.clear();
  }
}

func rep1(argument_0: dynamic, argument_1: dynamic)
{
      var k: dynamic;
      read(k);
      pos[k].pb(i);
      cnt[k] += 1;
      s.insert(k);
    }

func rep1(argument_0: dynamic, argument_1: dynamic)
{
      if ((cnt[i] == 0))
      {
        x = 1;
      }
      if (x)
      {
        ans.pb(0);
        continue;
      }
      var y = 0;
      var d = ((pos[i].back() - pos[i][0]) + 1);
      for (var id in pos[i])
      {
        var it = st.lower_bound(id);
        var pos2 = (*it);
        it -= 1;
        var pos1 = (*it);
        mn = min(mn, ((pos2 - pos1) - 1));
        if (((mn >= ((n - i) + 1)) && (mn >= d)))
        {
          y = 1;
        }
      }
      for (var id in pos[i])
      {
        st.insert(id);
      }
      if (y)
      {
        ans.pb(1);
      } else
      {
        ans.pb(0);
      }
    }
