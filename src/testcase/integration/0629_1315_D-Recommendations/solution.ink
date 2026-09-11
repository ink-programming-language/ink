// Translated from solution.cpp.

var MAX: dynamic = ((2e5) + 10);

var ara: dynamic = cpp_array(MAX);

var id: dynamic = cpp_array(MAX);

var didi: dynamic = cpp_array(MAX);

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      read(ara[i].first);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      read(ara[i].second);
      i += 1;
    }
  }
  sort(ara, (ara + n), __cpp_lambda_1);
  var cp: dynamic = -1;
  var que: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
  var prin: dynamic = cpp_uninitialized();
  sum = cpp_assign(prin, "=", 0);
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  var tc: dynamic = cpp_uninitialized();
  solve();
  return 0;
}

func __cpp_lambda_1(x: dynamic, y: dynamic) -> dynamic
{
  return  (((x.first == y.first))) ? (x.second > y.second) : (x.first < y.first);
}
