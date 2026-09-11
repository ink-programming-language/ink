// Translated from solution.cpp.

class query
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
}

var Q: dynamic = cpp_array(300001);

var arr: dynamic = cpp_array(300001);

var ans: dynamic = cpp_array(300001);

var freq: dynamic = cpp_array(300001);

var freqOfreq: dynamic = cpp_array(300001);

var currentMax: dynamic = 0;

var block: dynamic = 555;

func comp(a: dynamic, b: dynamic) -> dynamic
{
  if (((a.l / block) != (b.l / block)))
  {
    return ((a.l / block) < (b.l / block));
  }
  return (a.r < b.r);
}

func add(pos: dynamic) -> dynamic
{
  var x: dynamic = freq[arr[pos]];
  var y: dynamic = (x + 1);
  freq[arr[pos]] += 1;
  freqOfreq[x] -= 1;
  freqOfreq[y] += 1;
  if ((y > currentMax))
  {
    currentMax = y;
  }
}

func remove(pos: dynamic) -> dynamic
{
  var x: dynamic = freq[arr[pos]];
  var y: dynamic = (x - 1);
  freq[arr[pos]] -= 1;
  freqOfreq[x] -= 1;
  freqOfreq[y] += 1;
  if ((y < currentMax))
  {
    while ((freqOfreq[currentMax] == 0))
    {
      currentMax -= 1;
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, q);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      read(Q[i].l, Q[i].r);
      Q[i].l -= 1;
      Q[i].r -= 1;
      Q[i].i = i;
      i += 1;
    }
  }
  sort(Q, (Q + q), comp);
  var ML: dynamic = 0;
  var MR: dynamic = -1;
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var L: dynamic = Q[i].l;
      var R: dynamic = Q[i].r;
      while ((MR < R))
      {
        MR += 1;
        add(MR);
      }
      while ((ML > L))
      {
        ML -= 1;
        add(ML);
      }
      while ((MR > R))
      {
        remove(MR);
        MR -= 1;
      }
      while ((ML < L))
      {
        remove(ML);
        ML += 1;
      }
      var total: dynamic = ((Q[i].r - Q[i].l) + 1);
      var mx: dynamic = (((total + 1)) / 2);
      var rem: dynamic = (total - currentMax);
      if ((currentMax <= mx))
      {
        ans[Q[i].i] = 1;
      } else
      {
        ans[Q[i].i] = (total - (rem * 2));
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      write(ans[i], cpp_char("\n"));
      i += 1;
    }
  }
}
