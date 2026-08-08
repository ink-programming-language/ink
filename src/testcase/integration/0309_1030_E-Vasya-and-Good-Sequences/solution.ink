// Translated from solution.cpp.

var eps = (1e-9);

func dcmp(a: dynamic, b: dynamic)
{
  return if ((fabsl((a - b)) <= eps)) 0 else if (((a > b))) 1 else -1;
}

func getBit(num: dynamic, idx: dynamic)
{
  return (((((num >> idx)) & 1)) == 1);
}

func setBit1(num: dynamic, idx: dynamic)
{
  return (num | ((1 << idx)));
}

func setBit0(num: dynamic, idx: dynamic)
{
  return (num & (~((1 << idx))));
}

func flipBit(num: dynamic, idx: dynamic)
{
  return (num ^ ((1 << idx)));
}

func M()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

func countNumBit1(mask: dynamic)
{
  var ret = 0;
  while (mask)
  {
    mask &= ((mask - 1));
    ret += 1;
  }
  return ret;
}

var arr = cpp_array(300009);

var even = cpp_array(300009);

var odd = cpp_array(300009);

var v: dynamic;

var sum: dynamic;

func fun(no: dynamic)
{
  var cnt = 0;
  while (no)
  {
    cnt += ((no % 2));
    no /= 2;
  }
  return cnt;
}

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i]);
      var cnt = fun(arr[i]);
      v.push_back(cnt);
      i += 1;
    }
  }
  sum.resize(n);
  sum[0] = v[0];
  {
    var i = 1;
    while ((i < (cpp_cast((v).size()))))
    {
      sum[i] = (sum[(i - 1)] + v[i]);
      i += 1;
    }
  }
  even[0] = (((sum[0] % 2) == 0));
  odd[0] = (((sum[0] % 2) != 0));
  {
    var i = 1;
    while ((i < n))
    {
      even[i] = (even[(i - 1)] + (((sum[i] % 2) == 0)));
      odd[i] = (odd[(i - 1)] + (((sum[i] % 2) != 0)));
      i += 1;
    }
  }
  var add = 1;
  var rem = 0;
  var ans = 0;
  var res = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if (((rem % 2) == 0))
      {
        res += (even[(n - 1)] - even[((i + 1) - 1)]);
      } else
      {
        res += (odd[(n - 1)] - odd[((i + 1) - 1)]);
      }
      rem += v[i];
      var mx = v[i];
      var s = v[i];
      {
        var j = (i + 1);
        var k = 0;
        while (((j < n) && (k < 65)))
        {
          mx = max(mx, v[j]);
          s += v[j];
          if ((((s - mx) < mx) && ((s % 2) == 0)))
          {
            res -= 1;
          }
          j += 1;
          k += 1;
        }
      }
      i += 1;
    }
  }
  write(res, "\n");
}
