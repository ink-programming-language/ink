// Translated from solution.cpp.

var maxn = 5205;

var n: dynamic;

var sum = cpp_array(maxn, maxn);

var a: dynamic;

var cs = "0123456789ABCDEF";

var cidic: dynamic;

func init()
{
  {
    var i = 0;
    while ((i < 16))
    {
      cidic[cs[i]] = i;
      i += 1;
    }
  }
}

func update(i: dynamic, j: dynamic)
{
  sum[i][j] += (((if ((i > 0)) sum[(i - 1)][j] else 0) + (if ((j > 0)) sum[i][(j - 1)] else 0)) - (if (((i > 0) && (j > 0))) sum[(i - 1)][(j - 1)] else 0));
}

func check(k: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          var c_sum = (((sum[((i + k) - 1)][((j + k) - 1)] - (if ((j > 0)) sum[((i + k) - 1)][(j - 1)] else 0)) - (if ((i > 0)) sum[(i - 1)][((j + k) - 1)] else 0)) + (if (((i > 0) && (j > 0))) sum[(i - 1)][(j - 1)] else 0));
          if (((c_sum != 0) && (c_sum != (k * k))))
          {
            return false;
          }
          j += k;
        }
      }
      i += k;
    }
  }
  return true;
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  read(n);
  init();
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < (n / 4)))
        {
          read(a);
          var ms = cidic[a];
          sum[i][(j * 4)] = (((ms >> 3)) & 1);
          update(i, (j * 4));
          sum[i][((j * 4) + 1)] = (((ms >> 2)) & 1);
          update(i, ((j * 4) + 1));
          sum[i][((j * 4) + 2)] = (((ms >> 1)) & 1);
          update(i, ((j * 4) + 2));
          sum[i][((j * 4) + 3)] = (ms & 1);
          update(i, ((j * 4) + 3));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var k = 1;
  while (((k != n) && (!check((n / k)))))
  {
    k += 1;
    while (((n % k) != 0))
    {
      k += 1;
    }
  }
  write((n / k), "\n");
  return 0;
}
