// Translated from solution.cpp.

var MOD1: dynamic = (1e9 + 7);

var MOD2: dynamic = 998244353;

var INF: dynamic = LLONG_MAX;

var PI: dynamic = 3.14159265358979323846;

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func fpow(a: dynamic, b: dynamic, m: dynamic) -> dynamic
{
  if ((!b))
  {
    return 1;
  }
  var ans: dynamic = fpow(((a * a) % m), (b / 2), m);
  return ( ((b % 2)) ? ((ans * a) % m) : ans);
}

func inv(a: dynamic, m: dynamic) -> dynamic
{
  return fpow(a, (m - 2), m);
}

var MottoHayaku: dynamic = cpp_expression("#include <bits/stdc++.h> #in");

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll i=0;i<n;i++)");
}

func rep1(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll i=1;i<=n;i++)");
}

func repk(i: dynamic, m: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=m;i<n;i++)");
}

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

var pb: dynamic = cpp_expression("#include");

func SZ(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits");
}

func reset(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/st");
}

func cd(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <b");
}

func pow2(a: dynamic) -> dynamic
{
  return cpp_expression("#include");
}

func LB(a: dynamic, x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> #include");
}

func LPOS(a: dynamic, x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> #include <random> #");
}

func UB(a: dynamic, x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> #include");
}

func UPOS(a: dynamic, x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> #include <random> #");
}

func uni(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> #include <random> #include <ch");
}

func unisort(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> #incl");
}

var pos: dynamic = cpp_array(300005);

var cnt: dynamic = cpp_array(300005);

var s: dynamic = cpp_uninitialized();

var st: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var ll: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    rep1(i, n)[i].clear();
    cnt[i] = 0;
    st.insert(0);
    st.insert((n + 1));
    var x: dynamic = 0;
    var ans: dynamic = cpp_uninitialized();
    var mn: dynamic = INF;
    {
      var i: dynamic = (ans.size() - 1);
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

func rep1(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var k: dynamic = cpp_uninitialized();
      read(k);
      pos[k].pb(i);
      cnt[k] += 1;
      s.insert(k);
    }

func rep1(argument_0: dynamic, argument_1: dynamic) -> dynamic
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
      var y: dynamic = 0;
      var d: dynamic = ((pos[i].back() - pos[i][0]) + 1);
      for (var id: dynamic in pos[i])
      {
        var it: dynamic = st.lower_bound(id);
        var pos2: dynamic = (*it);
        it -= 1;
        var pos1: dynamic = (*it);
        mn = min(mn, ((pos2 - pos1) - 1));
        if (((mn >= ((n - i) + 1)) && (mn >= d)))
        {
          y = 1;
        }
      }
      for (var id: dynamic in pos[i])
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
