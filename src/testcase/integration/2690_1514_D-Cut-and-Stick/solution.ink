// Translated from solution.cpp.

class query
{
  var l: dynamic;
  var r: dynamic;
  var i: dynamic;
}

var Q = cpp_array(300001);

var arr = cpp_array(300001);

var ans = cpp_array(300001);

var freq = cpp_array(300001);

var freqOfreq = cpp_array(300001);

var currentMax = 0;

var block = 555;

func comp(a: dynamic, b: dynamic)
{
  if (((a.l / block) != (b.l / block)))
  {
    return ((a.l / block) < (b.l / block));
  }
  return (a.r < b.r);
}

func add(pos: dynamic)
{
  var x = freq[arr[pos]];
  var y = (x + 1);
  freq[arr[pos]] += 1;
  freqOfreq[x] -= 1;
  freqOfreq[y] += 1;
  if ((y > currentMax))
  {
    currentMax = y;
  }
}

func remove(pos: dynamic)
{
  var x = freq[arr[pos]];
  var y = (x - 1);
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);
  var n: dynamic;
  var q: dynamic;
  read(n, q);
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  {
    var i = 0;
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
  var ML = 0;
  var MR = -1;
  {
    var i = 0;
    while ((i < q))
    {
      var L = Q[i].l;
      var R = Q[i].r;
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
      var total = ((Q[i].r - Q[i].l) + 1);
      var mx = (((total + 1)) / 2);
      var rem = (total - currentMax);
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
    var i = 0;
    while ((i < q))
    {
      write(ans[i], cpp_char("\n"));
      i += 1;
    }
  }
}
