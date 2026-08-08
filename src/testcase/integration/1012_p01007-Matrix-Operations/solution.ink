// Translated from solution.cpp.

var arr = cpp_array(15, 15);

var t = cpp_array(15, 15);

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          assert(((i < 15) && (j < 15)));
          read(arr[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var o: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      read(o);
      var r: dynamic;
      var c: dynamic;
      var size: dynamic;
      var angle: dynamic;
      if ((o == 0))
      {
        read(r, c, size, angle);
        r -= 1;
        c -= 1;
        if (((angle == 0) || (angle == 360)))
        {
        } else if ((angle == 90))
        {
          memcpy(t, arr, cpp_sizeof(t));
          {
            var j = 0;
            while ((j < size))
            {
              {
                var k = 0;
                while ((k < size))
                {
                  assert((((r + j) < 15) && ((c + k) < 15)));
                  assert(((0 <= (r + j)) && (0 <= (c + k))));
                  assert((((((r + size) - 1) - k) < 15) && ((c + j) < 15)));
                  assert(((0 <= (((r + size) - 1) - k)) && (0 <= (c + j))));
                  arr[(r + j)][(c + k)] = t[(((r + size) - 1) - k)][(c + j)];
                  k += 1;
                }
              }
              j += 1;
            }
          }
        } else if ((angle == 180))
        {
          memcpy(t, arr, cpp_sizeof(t));
          {
            var j = 0;
            while ((j < size))
            {
              {
                var k = 0;
                while ((k < size))
                {
                  assert((((r + j) < 15) && ((c + k) < 15)));
                  assert((((((r + size) - 1) - j) < 15) && ((((c + size) - 1) - k) < 15)));
                  arr[(r + j)][(c + k)] = t[(((r + size) - 1) - j)][(((c + size) - 1) - k)];
                  k += 1;
                }
              }
              j += 1;
            }
          }
        } else if ((angle == 270))
        {
          memcpy(t, arr, cpp_sizeof(t));
          {
            var j = 0;
            while ((j < size))
            {
              {
                var k = 0;
                while ((k < size))
                {
                  assert((((r + j) < 15) && ((c + k) < 15)));
                  assert((((r + k) < 15) && ((((c + size) - 1) - j) < 15)));
                  arr[(r + j)][(c + k)] = t[(r + k)][(((c + size) - 1) - j)];
                  k += 1;
                }
              }
              j += 1;
            }
          }
        }
      } else if ((o == 1))
      {
        read(r, c, size);
        r -= 1;
        c -= 1;
        {
          var j = r;
          while ((j < (r + size)))
          {
            {
              var k = c;
              while ((k < (c + size)))
              {
                arr[j][k] = (1 - arr[j][k]);
                k += 1;
              }
            }
            j += 1;
          }
        }
      } else if ((o == 2))
      {
        read(r);
        r -= 1;
        rotate(arr[r], (arr[r] + 1), (arr[r] + n));
      } else if ((o == 3))
      {
        read(r);
        r -= 1;
        rotate(arr[r], ((arr[r] + n) - 1), (arr[r] + n));
      } else
      {
        read(r, c);
        r -= 1;
        c -= 1;
        var q: dynamic;
        q.push(make_pair(r, c));
        var v = (1 - arr[r][c]);
        while ((!q.empty()))
        {
          var nr = q.front().first;
          var nc = q.front().second;
          q.pop();
          if ((arr[nr][nc] == v))
          {
            continue;
          }
          arr[nr][nc] = v;
          if (((nr + 1) < n))
          {
            q.push(make_pair((nr + 1), nc));
          }
          if ((0 <= (nr - 1)))
          {
            q.push(make_pair((nr - 1), nc));
          }
          if (((nc + 1) < n))
          {
            q.push(make_pair(nr, (nc + 1)));
          }
          if ((0 <= (nc - 1)))
          {
            q.push(make_pair(nr, (nc - 1)));
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          write((if ((j == 0)) "" else " "), arr[i][j]);
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
  return 0;
}
