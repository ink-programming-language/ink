// Translated from solution.cpp.

class P
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
}

var v: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var e: dynamic = cpp_uninitialized();

var o: dynamic = cpp_array(112222);

var l: dynamic = cpp_array(2, 113222);

var j: dynamic = cpp_array(1);

var dx: dynamic = [0, 1, 0, -1, 1, 1, -1, -1];

var dy: dynamic = [1, 0, -1, 0, 1, -1, 1, -1];

var dz: dynamic = [0, 0, 0, 0, 1, -1];

var px: dynamic = [-1, 1, 1, -1, 1, 1, -1, -1];

var py: dynamic = [1, 1, -1, -1, 1, -1, 1, -1];

var mod: dynamic = 1000000007;

var mod2: dynamic = 1000000009;

var mod3: dynamic = 2017;

var x: dynamic = cpp_uninitialized();

var z: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var pi: dynamic = 3.14159265;

var u: dynamic = cpp_array(1151);

var u1: dynamic = cpp_array(1111);

var s: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var r: dynamic = cpp_array(2211);

var p: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d %d", (&a), (&b));
  {
    var t: dynamic = 1;
    while ((t <= a))
    {
      scanf("%d", (&n));
      {
        var w: dynamic = 1;
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
    var t: dynamic = 1;
    while ((t <= b))
    {
      scanf("%d %d", (&l[t][0]), (&n));
      {
        var w: dynamic = 1;
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
    var t: dynamic = 1;
    while ((t <= b))
    {
      {
        var w: dynamic = 0;
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
    var t: dynamic = 0;
    while ((t <= 1023))
    {
      if (u[t].x)
      {
        var k: dynamic = 0;
        {
          var w: dynamic = 1;
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
      var t: dynamic = 1;
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
