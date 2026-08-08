// Translated from solution.cpp.

var v1: dynamic;

var v2: dynamic;

var n: dynamic;

var s: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  read(n, s);
  var sum1 = 0;
  var sum2 = 0;
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      var y: dynamic;
      var z: dynamic;
      read(x, y, z);
      if ((y > z))
      {
        sum1 += x;
        ans += (x * y);
        v1.push_back(make_pair((y - z), x));
      } else
      {
        sum2 += x;
        ans += (x * z);
        v2.push_back(make_pair((z - y), x));
      }
      i += 1;
    }
  }
  var temp1 = 0;
  var temp2 = 0;
  if (((((sum1 % s) + (sum2 % s))) > s))
  {
    write(ans, "\n");
    return 0;
  }
  sum1 = (sum1 % s);
  sum2 = (sum2 % s);
  sort(v1.begin(), v1.end());
  {
    var i = 0;
    while ((i < v1.size()))
    {
      temp1 += (min(sum1, v1[i].second) * v1[i].first);
      sum1 -= min(sum1, v1[i].second);
      i += 1;
    }
  }
  sort(v2.begin(), v2.end());
  {
    var i = 0;
    while ((i < v2.size()))
    {
      temp2 += (min(sum2, v2[i].second) * v2[i].first);
      sum2 -= min(sum2, v2[i].second);
      i += 1;
    }
  }
  write((ans - min(temp1, temp2)), "\n");
  return 0;
}
