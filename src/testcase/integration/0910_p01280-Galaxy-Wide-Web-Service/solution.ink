// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < n; ++i)");
}

func rrep(i: dynamic, st: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = st; i < n; ++i)");
}

var inf: dynamic = (1e9 + 7);

var dy: dynamic = [0, 0, -1, 1, -1, 1, -1, 1];

var dx: dynamic = [1, -1, 0, 0, -1, 1, 1, -1];

func ceil(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream");
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<iost");
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<iost");
}

var n: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(101);

var t: dynamic = cpp_array(101);

var q: dynamic = cpp_array(101);

func ans_prime(p: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  var c: dynamic = 0;
  while ((cpp_update(c, "++") < p))
  {
    var tmp: dynamic = 0;
    chmax(ret, tmp);
  }
  return ret;
}

func main() -> dynamic
{
  while (((cin >> n) && n))
  {
    var c: dynamic = 0;
    var ans: dynamic = 0;
    while ((cpp_update(c, "++") < 55440))
    {
      var tmp: dynamic = 0;
      chmax(ans, tmp);
    }
    ans += ans_prime(13);
    ans += ans_prime(17);
    ans += ans_prime(19);
    ans += ans_prime(23);
    write(ans, "\n");
  }
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if ((d[i] == p))
      {
        tmp += q[i][t[i]];
        t[i] = ( (((t[i] + 1) == q[i].size())) ? 0 : (t[i] + 1));
      }
    }

func rep(argument_0: dynamic, i: dynamic) -> dynamic
{
        var q: dynamic = cpp_uninitialized();
        read(q);
        q[i].push_back(q);
      }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      q[i].resize(0);
      read(d[i], t[i]);
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        if (((((d[i] == 13) || (d[i] == 17)) || (d[i] == 23)) || (d[i] == 19)))
        {
          continue;
        }
        tmp += q[i][t[i]];
        t[i] = ( (((t[i] + 1) == q[i].size())) ? 0 : (t[i] + 1));
      }
