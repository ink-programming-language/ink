// Translated from solution.cpp.

func outarr(begin: dynamic, end: dynamic, delim: dynamic = " ")
{
  {
    var current = begin;
    while ((current != end))
    {
      write((*current), delim);
      current += 1;
    }
  }
  write(cpp_char("\n"));
}

var INF = 0x3f3f3f3f;

var MOD = static_cast((1e9 + 7));

class Segment
{
  var L: dynamic;
  var R: dynamic;
  var ID: dynamic;
}

func operator_less(lhs: dynamic, rhs: dynamic)
{
  if ((lhs.R == rhs.R))
  {
    return (lhs.L < rhs.L);
  }
  return (lhs.R < rhs.R);
}

var arr = cpp_array(100);

var ans = cpp_array(100);

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < (n)))
    {
      read(arr[i].L, arr[i].R);
      arr[i].ID = i;
      i += 1;
    }
  }
  sort(arr, (arr + n));
  var line: dynamic;
  {
    var i = 0;
    while ((i < (n)))
    {
      {
        var x = arr[i].L;
        while (true)
        {
          if ((line.find(x) == line.end()))
          {
            ans[arr[i].ID] = x;
            line.insert(x);
            break;
          }
          x += 1;
        }
      }
      i += 1;
    }
  }
  outarr(ans, (ans + n));
  return 0;
}
