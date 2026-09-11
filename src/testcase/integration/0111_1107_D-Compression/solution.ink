// Translated from solution.cpp.

var maxn: dynamic = 5205;

var n: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_array(maxn, maxn);

var a: dynamic = cpp_uninitialized();

var cs: dynamic = "0123456789ABCDEF";

var cidic: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 16))
    {
      cidic[cs[i]] = i;
      i += 1;
    }
  }
}

func update(i: dynamic, j: dynamic) -> dynamic
{
  sum[i][j] += ((( ((i > 0)) ? sum[(i - 1)][j] : 0) + ( ((j > 0)) ? sum[i][(j - 1)] : 0)) - ( (((i > 0) && (j > 0))) ? sum[(i - 1)][(j - 1)] : 0));
}

func check(k: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          var c_sum: dynamic = (((sum[((i + k) - 1)][((j + k) - 1)] - ( ((j > 0)) ? sum[((i + k) - 1)][(j - 1)] : 0)) - ( ((i > 0)) ? sum[(i - 1)][((j + k) - 1)] : 0)) + ( (((i > 0) && (j > 0))) ? sum[(i - 1)][(j - 1)] : 0));
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  read(n);
  init();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < (n / 4)))
        {
          read(a);
          var ms: dynamic = cidic[a];
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
  var k: dynamic = 1;
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
