// Translated from solution.cpp.

var eps: dynamic = (1e-9);

func dcmp(a: dynamic, b: dynamic) -> dynamic
{
  return  ((fabsl((a - b)) <= eps)) ? 0 :  (((a > b))) ? 1 : -1;
}

func getBit(num: dynamic, idx: dynamic) -> dynamic
{
  return (((((num >> idx)) & 1)) == 1);
}

func setBit1(num: dynamic, idx: dynamic) -> dynamic
{
  return (num | ((1 << idx)));
}

func setBit0(num: dynamic, idx: dynamic) -> dynamic
{
  return (num & (~((1 << idx))));
}

func flipBit(num: dynamic, idx: dynamic) -> dynamic
{
  return (num ^ ((1 << idx)));
}

func M() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

func countNumBit1(mask: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  while (mask)
  {
    mask &= ((mask - 1));
    ret += 1;
  }
  return ret;
}

var arr: dynamic = cpp_array(300009);

var even: dynamic = cpp_array(300009);

var odd: dynamic = cpp_array(300009);

var v: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

func fun(no: dynamic) -> dynamic
{
  var cnt: dynamic = 0;
  while (no)
  {
    cnt += ((no % 2));
    no /= 2;
  }
  return cnt;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(arr[i]);
      var cnt: dynamic = fun(arr[i]);
      v.push_back(cnt);
      i += 1;
    }
  }
  sum.resize(n);
  sum[0] = v[0];
  {
    var i: dynamic = 1;
    while ((i < (cpp_cast((v).size()))))
    {
      sum[i] = (sum[(i - 1)] + v[i]);
      i += 1;
    }
  }
  even[0] = (((sum[0] % 2) == 0));
  odd[0] = (((sum[0] % 2) != 0));
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      even[i] = (even[(i - 1)] + (((sum[i] % 2) == 0)));
      odd[i] = (odd[(i - 1)] + (((sum[i] % 2) != 0)));
      i += 1;
    }
  }
  var add: dynamic = 1;
  var rem: dynamic = 0;
  var ans: dynamic = 0;
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
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
      var mx: dynamic = v[i];
      var s: dynamic = v[i];
      {
        var j: dynamic = (i + 1);
        var k: dynamic = 0;
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
