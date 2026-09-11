// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

func setbit(n: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h>");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc+");
}

func clr(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

var fast: dynamic = cpp_expression("#include<bits/stdc++.h> using");

var endl: dynamic = cpp_expression("#inc");

var MOD: dynamic = cpp_expression("#include<b");

func dbg() -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h> using nam");
}

func logger(vars: dynamic, values: dynamic...) -> dynamic
{
  write(vars, " = ");
  var delim: dynamic = "";
  cpp_fold("(..., (cout << delim << values, delim = \",\"))");
  write("\n");
}

var inf: dynamic = 1e18;

func comp(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.second != b.second))
  {
    return (a.second < b.second);
  }
  return (a.first < b.first);
}

func main() -> dynamic
{
  var int_cpp: dynamic = 1;
  while (cpp_update(tt, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i].first, a[i].second);
        i += 1;
      }
    }
    sort(all(a), comp);
    pre[0] = a[0].first;
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        pre[i] = (pre[(i - 1)] + a[i].first);
        i += 1;
      }
    }
    var ans: dynamic = 0;
    var l: dynamic = 0;
    var r: dynamic = pre[(n - 1)];
    var check: dynamic = __cpp_lambda_1;
    while ((r >= l))
    {
      var mid: dynamic = (((l + r)) / 2);
      if (check(mid))
      {
        ans = mid;
        l = (mid + 1);
      } else
      {
        r = (mid - 1);
      }
    }
    write(((ans + (((pre[(n - 1)] - ans)) * 2))), "\n");
  }
  return 0;
}

func power(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while (b)
  {
    if ((b % 2))
    {
      b -= 1;
      res = (res * a);
    } else
    {
      b = (b / 2);
      a *= a;
    }
  }
  return res;
}

func __cpp_lambda_1(key: dynamic) -> dynamic
{
  var two: dynamic = ((pre[(n - 1)] - key));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((a[i].second > two))
      {
        return false;
      }
      two += a[i].first;
      if ((pre[i] >= key))
      {
        break;
      }
      i += 1;
    }
  }
  return true;
}
