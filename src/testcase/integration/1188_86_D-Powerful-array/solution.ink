// Translated from solution.cpp.

class query
{
  var l: dynamic;
  var r: dynamic;
  var block: dynamic;
  var id: dynamic;
}

var N = 200000;

var Q = 200000;

var n: dynamic;

var m: dynamic;

var a = cpp_array(N);

var freq = cpp_array(1000001);

var q = cpp_array(Q);

var ans = cpp_array(Q);

func by_block(a: dynamic, b: dynamic)
{
  if ((a.block != b.block))
  {
    return (a.block < b.block);
  }
  return (a.r > b.r);
}

func main()
{
  scanf("%d %d", (&n), (&m));
  var bsize = cpp_cast(sqrt(n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 0;
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
  var curr_l = 0;
  var curr_r = 0;
  var curr_ans = 0;
  {
    var i = 0;
    while ((i < m))
    {
      var l = q[i].l;
      var r = q[i].r;
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
    var i = 0;
    while ((i < m))
    {
      printf("%I64d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
