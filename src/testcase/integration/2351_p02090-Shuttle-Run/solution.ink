// Translated from solution.cpp.

var vi: dynamic = cpp_expression("#include <b");

var int_cpp: dynamic = dynamic;

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < n; i++)");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.");
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var data: dynamic = cpp_construct((n * 2));
  sort(all(data));
  rep(i, data.size());
  {
    if (i)
    {
      data[i].second += data[(i - 1)].second;
    }
  }
  var mx: dynamic = -1;
  rep(i, data.size());
  {
    if ((mx < data[i].second))
    {
      mx = data[i].second;
    }
  }
  if ((mx % 2))
  {
    var ans: dynamic = 0;
    {
      var i: dynamic = (data.size() - 1);
      while ((i >= 0))
      {
        if ((data[ans].second < data[i].second))
        {
          ans = i;
        }
        i -= 1;
      }
    }
    write(((((mx - 1)) * m) + ((data[(ans + 1)].first - 1))), "\n");
  } else
  {
    var ans: dynamic = (data.size() - 1);
    rep(i, data.size());
    {
      if ((data[ans].second < data[i].second))
      {
        ans = i;
      }
    }
    write(((((mx - 1)) * m) + ((m - data[ans].first))), "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    read(l, r);
    data[(i * 2)] = [l, 1];
    data[((i * 2) + 1)] = [(r + 1), -1];
  }
