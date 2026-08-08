// Translated from solution.cpp.

class P
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
}

var v: dynamic;

var i: dynamic;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var a: dynamic;

var d: dynamic;

var b: dynamic;

var c: dynamic;

var e: dynamic;

var o = cpp_array(112222);

var l = cpp_array(2, 113222);

var j = cpp_array(1);

var dx = [0, 1, 0, -1, 1, 1, -1, -1];

var dy = [1, 0, -1, 0, 1, -1, 1, -1];

var dz = [0, 0, 0, 0, 1, -1];

var px = [-1, 1, 1, -1, 1, 1, -1, -1];

var py = [1, 1, -1, -1, 1, -1, 1, -1];

var mod = 1000000007;

var mod2 = 1000000009;

var mod3 = 2017;

var x: dynamic;

var z: dynamic;

var y: dynamic;

var pi = 3.14159265;

var u = cpp_array(1151);

var u1 = cpp_array(1111);

var s: dynamic;

var q: dynamic;

var r = cpp_array(2211);

var p: dynamic;

func main()
{
  scanf("%d %d", (&a), (&b));
  {
    var t = 1;
    while ((t <= a))
    {
      scanf("%d", (&n));
      {
        var w = 1;
        while ((w <= n))
        {
          scanf("%d", (&i));
          o[t] |= ((1 << ((i - 1))));
          w += 1;
        }
      }
      t += 1;
    }
  }
  {
    var t = 1;
    while ((t <= b))
    {
      scanf("%d %d", (&l[t][0]), (&n));
      {
        var w = 1;
        while ((w <= n))
        {
          scanf("%d", (&i));
          l[t][1] |= ((1 << ((i - 1))));
          w += 1;
        }
      }
      if (((u1[l[t][1]].x == 0) || (u1[l[t][1]].x > l[t][0])))
      {
        u1[l[t][1]] = [l[t][0], t];
      }
      t += 1;
    }
  }
  {
    var t = 1;
    while ((t <= b))
    {
      {
        var w = 0;
        while ((w <= 1023))
        {
          if ((((u1[w].y != t) && u1[w].y) && (((u[(l[t][1] | w)].x == 0) || (u[(l[t][1] | w)].x > (l[t][0] + u1[w].x))))))
          {
            u[(l[t][1] | w)] = [(l[t][0] + u1[w].x), u1[w].y, t];
          }
          w += 1;
        }
      }
      t += 1;
    }
  }
  n = 0;
  m = 987654321;
  i = 987654321;
  {
    var t = 0;
    while ((t <= 1023))
    {
      if (u[t].x)
      {
        var k = 0;
        {
          var w = 1;
          while ((w <= a))
          {
            if ((((t & o[w])) == o[w]))
            {
              k += 1;
            }
            w += 1;
          }
        }
        if (((k > n) || (((k == n) && (m > u[t].x)))))
        {
          n = k;
          m = u[t].x;
          i = t;
        }
      }
      t += 1;
    }
  }
  if ((i == 987654321))
  {
    n = 1987654321;
    m = 1987654321;
    {
      var t = 1;
      while ((t <= b))
      {
        if ((n > l[t][0]))
        {
          m = n;
          d = c;
          n = l[t][0];
          c = t;
        } else if ((m > l[t][0]))
        {
          m = l[t][0];
          d = t;
        }
        t += 1;
      }
    }
    printf("%d %d", c, d);
  } else
  {
    printf("%d %d", u[i].y, u[i].z);
  }
}
