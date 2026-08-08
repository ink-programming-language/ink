// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var data = cpp_array(100010, 2);

var t = cpp_array(100010);

var num = cpp_array(100010);

var que: dynamic;

func dis(x: dynamic, y: dynamic)
{
  return abs((data[0][x] - data[1][y]));
}

func main()
{
  scanf("%d%d%*d%*d", (&n), (&m));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&data[0][i]));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      scanf("%d", (&data[1][i]));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      t[i] = 0x7fffffff;
      i += 1;
    }
  }
  var j = 0;
  {
    var i = 0;
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
      var k = j;
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
    var i = que.front().first;
    var j = que.front().second;
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
  var ans = n;
  {
    var i = 0;
    while ((i < m))
    {
      ans -= num[i];
      i += 1;
    }
  }
  printf("%d\n", ans);
}
