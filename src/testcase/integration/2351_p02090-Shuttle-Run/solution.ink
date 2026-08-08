// Translated from solution.cpp.

var vi = cpp_expression("#include <b");

var int_cpp = dynamic;

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < n; i++)");
}

func all(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.");
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var data = cpp_construct((n * 2));
  sort(all(data));
  rep(i, data.size());
  {
    if (i)
    {
      data[i].second += data[(i - 1)].second;
    }
  }
  var mx = -1;
  rep(i, data.size());
  {
    if ((mx < data[i].second))
    {
      mx = data[i].second;
    }
  }
  if ((mx % 2))
  {
    var ans = 0;
    {
      var i = (data.size() - 1);
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
    var ans = (data.size() - 1);
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var l: dynamic;
    var r: dynamic;
    read(l, r);
    data[(i * 2)] = [l, 1];
    data[((i * 2) + 1)] = [(r + 1), -1];
  }
