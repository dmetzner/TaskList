export function validateName(name) {
    return name === undefined || name == null || name.length < 1 || name.length > 30;
}

export function validateDescription(desc) {
    return desc === undefined || desc === null || desc.length < 0 || desc.length > 100;
}

export function validateTags(tags) {
    return tags === undefined || tags === null;
}

export function validateParsedTags(tags) {
    for (let i = 0; i < tags.length; i++) {
        if (tags[i] == null || tags[i].length < 1 || tags[i].length > 30) {
            return true
        }
    }
    return false
}


const escape_sign = "%";
const delimiter = ",";

function unescapeTag(tag) {
    let escaped = false;
    let unescaped_tag = "";
    for (let i = 0; i < tag.length; i++) {
        let c = tag[i];
        if (!escaped && c === escape_sign) {
            escaped = true;
        } else {
            unescaped_tag += c;
            escaped = false;
        }
    }
    return unescaped_tag;
}


export function escapeTag(tag) {
    let escaped_tag = "";
    for (let i = 0; i < tag.length; i++) {
        let c = tag[i];
        if (c === escape_sign || c === delimiter) {
            escaped_tag += escape_sign;
            escaped_tag += c;
        } else {
            escaped_tag += c;
        }
    }
    return escaped_tag;
}

export function parseAndUnEscapeTags(tags_as_string) {

    let tags = [];
    let tag = "";
    let escaped = false;

    for (let i = 0; i < tags_as_string.length; i++) {

        let c = tags_as_string[i];
        if (escaped) {

            if (c === escape_sign || c === delimiter) {
                // if escaped -> only allow escaped chars!
                tag += "%";
                tag += c;
            } else {
                // error!
                break;
            }
            escaped = false;
        } else {
            if (c === escape_sign) {
                escaped = true;
            } else if (c === delimiter) {
                // add tag and start a new one
                tags.push(unescapeTag(tag));
                tag = "";
            } else {
                tag += c;
            }
        }
    }

    if (escaped) {
        // still escaped? we had an error!
        return null;
    }

    // also add the last tag!
    tags.push(unescapeTag(tag));

    return tags;
}

