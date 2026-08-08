// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  read(n);
  var point = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(point[i].first, point[i].second);
      i += 1;
    }
  }
  sort(point, (point + n), __cpp_lambda_1);
  var ans = 0;
  var cnt = 1;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      if ((point[i].first == point[(i + 1)].first))
      {
        cnt += 1;
      } else
      {
        ans += ((cnt * ((cnt - 1))) / 2);
        cnt = 1;
      }
      i += 1;
    }
  }
  ans += ((cnt * ((cnt - 1))) / 2);
  sort(point, (point + n), __cpp_lambda_2);
  cnt = 1;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      if ((point[i].second == point[(i + 1)].second))
      {
        cnt += 1;
      } else
      {
        ans += ((cnt * ((cnt - 1))) / 2);
        cnt = 1;
      }
      i += 1;
    }
  }
  ans += ((cnt * ((cnt - 1))) / 2);
  sort(point, (point + n));
  cnt = 1;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      if ((point[i] == point[(i + 1)]))
      {
        cnt += 1;
      } else
      {
        ans -= ((cnt * ((cnt - 1))) / 2);
        cnt = 1;
      }
      i += 1;
    }
  }
  ans -= ((cnt * ((cnt - 1))) / 2);
  write(ans, cpp_char("\n"));
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (a.first < b.first);
}

func __cpp_lambda_2(a: dynamic, b: dynamic)
{
  return (a.second < b.second);
}
