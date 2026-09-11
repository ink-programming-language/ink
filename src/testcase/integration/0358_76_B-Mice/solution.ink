// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var data: dynamic = cpp_array(100010, 2);

var t: dynamic = cpp_array(100010);

var num: dynamic = cpp_array(100010);

var que: dynamic = cpp_uninitialized();

func dis(x: dynamic, y: dynamic) -> dynamic
{
  return abs((data[0][x] - data[1][y]));
}

func main() -> dynamic
{
  scanf("%d%d%*d%*d", (&n), (&m));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&data[0][i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d", (&data[1][i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      t[i] = 0x7fffffff;
      i += 1;
    }
  }
  var j: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      while ((((j + 1) < m) && (dis(i, (j + 1)) < dis(i, j))))
      {
        j += 1;
      }
      if ((((j + 1) < m) && (dis(i, j) == dis(i, (j + 1)))))
      {
        que.push(make_pair(i, j));
        i += 1;
        continue;
      }
      var k: dynamic = j;
      if ((((j + 1) < m) && (dis(i, (j + 1)) < dis(i, j))))
      {
        k += 1;
      }
      if ((t[k] > dis(i, k)))
      {
        num[k] = 0;
        t[k] = dis(i, k);
      }
      if ((t[k] == dis(i, k)))
      {
        num[k] += 1;
      }
      i += 1;
    }
  }
  while ((!que.empty()))
  {
    var i: dynamic = que.front().first;
    var j: dynamic = que.front().second;
    que.pop();
    if (((dis(i, j) == t[j]) || (t[j] == 0x7fffffff)))
    {
      num[j] += 1;
      t[j] = dis(i, j);
    } else if (((dis(i, (j + 1)) == t[(j + 1)]) || (t[(j + 1)] == 0x7fffffff)))
    {
      num[(j + 1)] += 1;
      t[(j + 1)] = dis(i, (j + 1));
    }
  }
  var ans: dynamic = n;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      ans -= num[i];
      i += 1;
    }
  }
  printf("%d\n", ans);
}
