

def validate_name(name):
    # return true if invalid
    return name is None or len(name) < 1 or len(name) > 30


def validate_description(desc):
    # return true if invalid
    return desc is None or len(desc) < 0 or len(desc) > 100


def validate_tags(tags):
    # return true if invalid
    return tags is None


def validate_parsed_tags(tags):
    # return true if invalid
    for tag_name in tags:
        if validate_name(tag_name):
            return True

    return False


escape_sign = "%"
delimiter = ","


def unescape_tag(tag):
    escaped = False
    unescaped_tag = ""

    for c in tag:
        if not escaped and c == escape_sign:
            escaped = True

        else:
            unescaped_tag += c
            escaped = False

    return unescaped_tag


def escape_tag(tag):
    escaped_tag = ""
    for c in tag:
        if c == escape_sign or c == delimiter:
            escaped_tag += escape_sign
            escaped_tag += c
        else :
            escaped_tag += c

    return escaped_tag


def parse_tags(tags_as_string):

    tags = []
    tag = ""
    escaped = False

    for c in tags_as_string:

        if escaped:
            if c == escape_sign or c == delimiter:
                # if escaped -> only allow escaped chars!
                tag += escape_sign
                tag += c
            else:
                # error!
                break
            
            escaped = False
        
        else:
            if c == escape_sign:
                escaped = True
            
            elif c == delimiter:
                # add tag and start a new one
                tags.append(unescape_tag(tag))
                tag = ""
            else:
                tag += c

    if escaped:
        # still escaped? we had an error!
        return None

    # also add the last tag!
    tags.append(unescape_tag(tag))

    return tags
