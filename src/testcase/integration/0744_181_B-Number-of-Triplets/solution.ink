// Translated from solution.cpp.

class diem
{
  var x: dynamic;
  var y: dynamic;
}

var n: dynamic;

var ans = 0;

var a = cpp_array(3005);

func cmp(a: dynamic, b: dynamic)
{
  if ((a.x == b.x))
  {
    return (a.y <= b.y);
  }
  return (a.x <= b.x);
}

func binary_search(l: dynamic, r: dynamic, value: dynamic)
{
  if ((l > r))
  {
    return 0;
  }
  if ((l == r))
  {
    return l;
  }
  var mid = (((l + r)) / 2);
  if (cmp(value, a[mid]))
  {
    return binary_search(l, mid, value);
  } else
  {
    return binary_search((mid + 1), r, value);
  }
}

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i].x, a[i].y);
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1), cmp);
  {
    var i = 1;
    while ((i <= (n - 2)))
    {
      {
        var j = (i + 2);
        while ((j <= n))
        {
          if ((((abs((a[j].x - a[i].x)) % 2) == 0) && ((abs((a[j].y - a[i].y)) % 2) == 0)))
          {
            var temp: dynamic;
            temp.x = (((a[j].x + a[i].x)) / 2);
            temp.y = (((a[j].y + a[i].y)) / 2);
            var vt = binary_search(i, j, temp);
            if (((a[vt].x == temp.x) && (a[vt].y == temp.y)))
            {
              ans += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans);
}
