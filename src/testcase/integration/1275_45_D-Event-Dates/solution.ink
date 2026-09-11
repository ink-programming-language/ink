// Translated from solution.cpp.

func outarr(begin: dynamic, end: dynamic, delim: dynamic = " ") -> dynamic
{
  {
    var current: dynamic = begin;
    while ((current != end))
    {
      write((*current), delim);
      current += 1;
    }
  }
  write(cpp_char("\n"));
}

var INF: dynamic = 0x3f3f3f3f;

var MOD: dynamic = static_cast((1e9 + 7));

class Segment
{
  var L: dynamic = cpp_uninitialized();
  var R: dynamic = cpp_uninitialized();
  var ID: dynamic = cpp_uninitialized();
}

func operator_less(lhs: dynamic, rhs: dynamic) -> dynamic
{
  if ((lhs.R == rhs.R))
  {
    return (lhs.L < rhs.L);
  }
  return (lhs.R < rhs.R);
}

var arr: dynamic = cpp_array(100);

var ans: dynamic = cpp_array(100);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      read(arr[i].L, arr[i].R);
      arr[i].ID = i;
      i += 1;
    }
  }
  sort(arr, (arr + n));
  var line: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      {
        var x: dynamic = arr[i].L;
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
