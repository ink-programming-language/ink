// Translated from solution.cpp.

var INF: dynamic = 500000000;

func debug(a: dynamic, b: dynamic) -> dynamic
{
  {
    while ((a != b))
    {
      write((*a), cpp_char(" "));
      a += 1;
    }
  }
  write("\n");
}

var w: dynamic = cpp_uninitialized();

var h: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var buf: dynamic = cpp_array(55, 55);

var moveTo: dynamic = cpp_array(2, 4, 55, 55);

var state: dynamic = cpp_array(2, 4, 55, 55);

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

func main() -> dynamic
{
  read(h, n);
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      read(buf[i]);
      i += 1;
    }
  }
  w = strlen(buf[0]);
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      {
        var j: dynamic = 0;
        while ((j < w))
        {
          {
            var k: dynamic = 0;
            while ((k < 4))
            {
              {
                var l: dynamic = 0;
                while ((l < 2))
                {
                  var cx: dynamic = j;
                  var cy: dynamic = i;
                  while (1)
                  {
                    var px: dynamic = (cx + dx[k]);
                    var py: dynamic = (cy + dy[k]);
                    if ((((((px < 0) || (py < 0)) || (px >= w)) || (py >= h)) || (buf[py][px] != buf[cy][cx])))
                    {
                      break;
                    }
                    cx = px;
                    cy = py;
                  }
                  var add: dynamic = cpp_uninitialized();
                  if ((l == 0))
                  {
                    add = 3;
                  } else
                  {
                    add = 1;
                  }
                  var k2: dynamic = (((k + add)) % 4);
                  var px: dynamic = cpp_uninitialized();
                  var py: dynamic = cpp_uninitialized();
                  while (1)
                  {
                    px = (cx + dx[k2]);
                    py = (cy + dy[k2]);
                    if ((((((px < 0) || (py < 0)) || (px >= w)) || (py >= h)) || (buf[py][px] != buf[cy][cx])))
                    {
                      break;
                    }
                    cx = px;
                    cy = py;
                  }
                  px = (cx + dx[k]);
                  py = (cy + dy[k]);
                  if ((((((px < 0) || (py < 0)) || (px >= w)) || (py >= h)) || (buf[py][px] == cpp_char("0"))))
                  {
                    state[i][j][k][l] = 1;
                    moveTo[i][j][k][l] = make_pair(cy, cx);
                  } else
                  {
                    moveTo[i][j][k][l] = make_pair(py, px);
                  }
                  l += 1;
                }
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var cx: dynamic = 0;
  var cy: dynamic = 0;
  var dir: dynamic = 0;
  var hand: dynamic = 0;
  {
    var hoge: dynamic = 0;
    while ((hoge < n))
    {
      var nxt: dynamic = moveTo[cy][cx][dir][hand];
      var flag: dynamic = state[cy][cx][dir][hand];
      cy = nxt.first;
      cx = nxt.second;
      if (flag)
      {
        if ((hand == 0))
        {
          hand = 1;
        } else
        {
          hand = 0;
          dir = (((dir + 1)) % 4);
        }
      }
      hoge += 1;
    }
  }
  printf("%c\n", buf[cy][cx]);
  return 0;
}
