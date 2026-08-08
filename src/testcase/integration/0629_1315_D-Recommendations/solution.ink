// Translated from solution.cpp.

var MAX = ((2e5) + 10);

var ara = cpp_array(MAX);

var id = cpp_array(MAX);

var didi = cpp_array(MAX);

func solve()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i <= (n - 1)))
    {
      read(ara[i].first);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= (n - 1)))
    {
      read(ara[i].second);
      i += 1;
    }
  }
  sort(ara, (ara + n), __cpp_lambda_1);
  var cp = -1;
  var que: dynamic;
  var sum: dynamic;
  var prin: dynamic;
  sum = cpp_assign(prin, "=", 0);
  {
    var i = 0;
    while ((i <= (n - 1)))
    {
      if ((cp >= ara[i].first))
      {
        que.push(ara[i].second);
        sum += ara[i].second;
      } else
      {
        if ((!que.empty()))
        {
          sum -= que.top();
          que.pop();
          prin += sum;
          cp += 1;
          i -= 1;
        } else
        {
          cp = ara[i].first;
          i -= 1;
        }
      }
      i += 1;
    }
  }
  while ((!que.empty()))
  {
    sum -= que.top();
    que.pop();
    prin += sum;
  }
  write(prin, "\n");
}

func main()
{
  ios.sync_with_stdio(false);
  var tc: dynamic;
  solve();
  return 0;
}

func __cpp_lambda_1(x: dynamic, y: dynamic)
{
  return if (((x.first == y.first))) (x.second > y.second) else (x.first < y.first);
}
