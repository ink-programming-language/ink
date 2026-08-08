// Translated from solution.cpp.

var Maxn = ((100 * 1000) + 10);

var n: dynamic;

var inda = cpp_array(Maxn);

var indb = cpp_array(Maxn);

var a: dynamic;

var b: dynamic;

var b1: dynamic;

func main()
{
  scanf("%d", (&n));
  var aa: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&aa));
      aa -= 1;
      inda[aa] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&aa));
      aa -= 1;
      indb[aa] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((indb[i] > inda[i]))
      {
        a.insert(make_pair((indb[i] - inda[i]), inda[i]));
      } else
      {
        b.insert(make_pair((inda[i] - indb[i]), indb[i]));
        b1.insert(make_pair(indb[i], (inda[i] - indb[i])));
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      printf("%d\n", min((((b.begin()->first + i)) % n), ((((a.begin()->first - i) + n)) % n)));
      if ((b1.begin()->first == i))
      {
        var ind = (b1.begin()->second + i);
        a.insert(make_pair(((n - ind) + i), ind));
        b.erase(make_pair(b1.begin()->second, b1.begin()->first));
        b1.erase(b1.begin());
      }
      while (((!a.empty()) && (a.begin()->first == (i + 1))))
      {
        b.insert(make_pair(((-i) - 1), ((a.begin()->second + i) + 1)));
        b1.insert(make_pair(((a.begin()->second + i) + 1), ((-i) - 1)));
        a.erase(a.begin());
      }
      i += 1;
    }
  }
  return 0;
}
