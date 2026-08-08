// Translated from solution.cpp.

var nax = ((100 * 1000) + 10);

var n: dynamic;

var m: dynamic;

var l = cpp_array(nax);

var p = cpp_array(nax);

var sum: dynamic;

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(n, m);
  {
    var i = 1;
    while ((i <= m))
    {
      read(l[i]);
      sum += l[i];
      i += 1;
    }
  }
  if ((sum < n))
  {
    write("-1");
    return 0;
  }
  {
    var i = 1;
    while ((i <= m))
    {
      if ((((l[i] + i) - 1) > n))
      {
        write("-1");
        return 0;
      }
      p[i] = i;
      i += 1;
    }
  }
  var w = n;
  var pos = m;
  while ((pos > 0))
  {
    if ((((p[pos] + l[pos]) - 1) < w))
    {
      p[pos] = ((w - l[pos]) + 1);
      w = (p[pos] - 1);
      pos -= 1;
    } else
    {
      break;
    }
  }
  if ((p[1] != 1))
  {
    write("-1");
    return 0;
  }
  {
    var i = 1;
    while ((i <= m))
    {
      write(p[i], " ");
      i += 1;
    }
  }
}
