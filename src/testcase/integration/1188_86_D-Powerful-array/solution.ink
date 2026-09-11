// Translated from solution.cpp.

class query
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var block: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var N: dynamic = 200000;

var Q: dynamic = 200000;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var freq: dynamic = cpp_array(1000001);

var q: dynamic = cpp_array(Q);

var ans: dynamic = cpp_array(Q);

func by_block(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.block != b.block))
  {
    return (a.block < b.block);
  }
  return (a.r > b.r);
}

func main() -> dynamic
{
  scanf("%d %d", (&n), (&m));
  var bsize: dynamic = cpp_cast(sqrt(n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d %d", (&q[i].l), (&q[i].r));
      q[i].l -= 1;
      q[i].block = (q[i].l / bsize);
      q[i].id = i;
      i += 1;
    }
  }
  sort(q, (q + m), by_block);
  var curr_l: dynamic = 0;
  var curr_r: dynamic = 0;
  var curr_ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var l: dynamic = q[i].l;
      var r: dynamic = q[i].r;
      while ((curr_l < l))
      {
        curr_ans += ((ll)((1 - (2 * freq[a[curr_l]]))) * a[curr_l]);
        freq[a[curr_l]] -= 1;
        curr_l += 1;
      }
      while ((curr_l > l))
      {
        curr_ans += ((ll)((1 + (2 * freq[a[(curr_l - 1)]]))) * a[(curr_l - 1)]);
        freq[a[(curr_l - 1)]] += 1;
        curr_l -= 1;
      }
      while ((curr_r < r))
      {
        curr_ans += ((ll)((1 + (2 * freq[a[curr_r]]))) * a[curr_r]);
        freq[a[curr_r]] += 1;
        curr_r += 1;
      }
      while ((curr_r > r))
      {
        curr_ans += ((ll)((1 - (2 * freq[a[(curr_r - 1)]]))) * a[(curr_r - 1)]);
        freq[a[(curr_r - 1)]] -= 1;
        curr_r -= 1;
      }
      ans[q[i].id] = curr_ans;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      printf("%I64d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
